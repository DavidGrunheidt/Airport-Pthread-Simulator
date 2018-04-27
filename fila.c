#include "fila.h"

/**
 * fila.c
 * Implementação das funções da fila.h
 * Descrições em fila.h
 **/

elemento_t * aloca_elemento (aviao_t * dado) {
	elemento_t *elemento = (elemento_t *) malloc(sizeof (elemento_t));
	elemento->dado = dado;
	return elemento;
}

void desaloca_elemento (elemento_t * elemento) {
	free(elemento->dado);
	free(elemento);
}

fila_ordenada_t * criar_fila () {
	fila_ordenada_t *fila = (fila_ordenada_t *) malloc(sizeof(fila_ordenada_t));

	pthread_mutex_init (&fila->entraFila, NULL);
	pthread_mutex_init (&fila->saiFila, NULL);
	fila->primeiro = NULL;
	fila->ultimo = NULL;

	fila->n_elementos = 0;

	return fila;
}

void desaloca_fila (fila_ordenada_t * fila) {
	pthread_mutex_lock(&fila->saiFila);
	pthread_mutex_unlock(&fila->entraFila);
	elemento_t *elemento = fila->primeiro->proximo;
	while (elemento != fila->ultimo) {
		desaloca_elemento(elemento);
		elemento = elemento->proximo;
	}
	desaloca_elemento(elemento);
	pthread_mutex_unlock(&fila->entraFila);
	pthread_mutex_unlock(&fila->saiFila);

	pthread_mutex_destroy(&fila->entraFila);
	pthread_mutex_destroy(&fila->saiFila);
	free(fila);
}

void inserir (fila_ordenada_t * fila, aviao_t * dado) {
	elemento_t *elemento = aloca_elemento(dado);

	if (dado->combustivel < 10) {
		pthread_mutex_lock(&fila->saiFila);
		fila->primeiro->anterior = elemento;
		elemento->proximo = fila->primeiro;
		elemento->anterior = NULL;
		fila->primeiro = elemento;
		fila->n_elementos++;
		pthread_mutex_unlock(&fila->saiFila);
	} else {
		pthread_mutex_lock(&fila->entraFila);
		fila->ultimo->proximo = elemento;
		elemento->anterior = fila->ultimo;
		elemento->proximo = NULL;
		fila->n_elementos++;
		pthread_mutex_lock(&fila->entraFila);
	}
}

// Mudar pra achar o aviao em vez do index
aviao_t * remover (fila_ordenada_t * fila, aviao_t *aviao) {
	if (1) {
		pthread_mutex_lock(&fila->saiFila);
		elemento_t *elementoRemovido = fila->primeiro;
		for (int i = 1; i < fila->n_elementos; i++) 
			elementoRemovido = elementoRemovido->proximo;

		if (index != 1)
			elementoRemovido->anterior->proximo = elementoRemovido->proximo;
		else
			fila->primeiro = elementoRemovido->proximo;

		elementoRemovido->proximo->anterior = elementoRemovido->anterior;

		desaloca_elemento(elementoRemovido);
		fila->n_elementos--;
		pthread_mutex_unlock(&fila->saiFila);
	} else {
		if (index == fila->n_elementos) {
			pthread_mutex_lock(&fila->entraFila);
			elemento_t *elementoRemovido = fila->ultimo;

			fila->ultimo = elementoRemovido->anterior;
			fila->ultimo->proximo = NULL;
			desaloca_elemento(elementoRemovido);
			fila->n_elementos--;
			pthread_mutex_unlock(&fila->entraFila);
		}
	}
}

size_t at (fila_ordenada_t * fila, aviao_t *aviao) {
	int index = 0;

	elemento_t *elementoAux = fila->primeiro;

	for (int i = 1; i <= fila->n_elementos; i++) {
		if (i+1 == fila->n_elementos)
			pthread_mutex_lock(&fila->saiFila);


		if (elementoAux->dado->id = aviao->id) {
			index = i;
			break;
		} else {
			if (i+1 == fila->n_elementos) {
				pthread_mutex_lock(&fila->saiFila);
				elementoAux = elementoAux->proximo;
			}
		}
		if (i == fila->n_elementos)
			pthread_mutex_unlock(&fila->saiFila);
	}
	return index;
}