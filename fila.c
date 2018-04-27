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

	pthread_mutex_init (&fila->mutexFila, NULL);
	fila->primeiro = NULL;
	fila->ultimo = NULL;

	fila->n_elementos = 0;

	return fila;
}

void desaloca_fila (fila_ordenada_t * fila) {
	pthread_mutex_lock(&fila->mutexFila);

	elemento_t *elemento = fila->primeiro->proximo;
	while (elemento != fila->ultimo) {
		desaloca_elemento(elemento);
		elemento = elemento->proximo;
	}
	desaloca_elemento(elemento);

	pthread_mutex_unlock(&fila->mutexFila);

	pthread_mutex_destroy(&fila->mutexFila);
	free(fila);
}

void inserir (fila_ordenada_t * fila, aviao_t * dado, size_t index) {
	elemento_t *elemento = aloca_elemento(dado);
	if (index <= fila->n_elementos) {
		if (index == 1) {
			inserirPrimeiro(fila, aviao);
		} else {
			elemento_t *elementoAux = fila->primeiro;
			for (int i = 1; i < index; i++)
				elementoAux = elementoAux->proximo;
			elemento->anterior = elementoAux->anterior;
			elemento->proximo = elementoAux;
			elementoAux->anterior->proximo = elemento;
			elementoAux->anterior = elemento;
			fila->n_elementos++;
		}
	}
}

}

void inserirPrimeiro(fila_ordenada_t *fila, aviao_t *dado) {
	elemento->anterior = NULL;
	elemento->proximo = fila->primeiro;
	fila->primeiro->anterior = elemento;
	fila->primeiro = elemento;
	fila->n_elementos++;
}

void inserirUltimo(fila_ordenada_t *fila, aviao_t *dado, int lock) {
	elemento->proximo = NULL;
	if (lock == 1)
		pthread_mutex_lock(&fila->mutexFila);
	elemento->anterior = fila->ultimo;
	fila->ultimo->proximo = elemento;
	fila->ultimo = elemento;
	fila->n_elementos++;
	if (lock == 1)
		pthread_mutex_unlock(&fila->mutexFila);

}

aviao_t * remover (fila_ordenada_t * fila) {
	// Não é necessario dar lock no mutex, já é feito no main
	aviao_t *aviao;

	elemento_t *elementoRemovido = fila->primeiro;
	fila->primeiro = fila->primeiro->proximo;
	aviao = elementoRemovido->dado;

	// Somente o destravamento é necessario visto que só é possivel
	// chegar nesta função após travar o mutex no main e
	// verificar se o aviao é o primeiro
	pthread_mutex_unlock(&fila->mutexFila);

	return aviao;
}