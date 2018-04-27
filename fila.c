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
	elemento->proximo = NULL;

	// Só uma thread por vez mexendo no final da fila
	pthread_mutex_lock(&fila->entraFila);
	if (fila->n_elementos == 0) {
		fila->primeiro = elemento;
		fila->ultimo = elemento;
	} else {
		// Priodade de pouso com combustivel inferior a 10%
		// Adiciona na primeira posição da fila
		if (dado->combustivel < 10) {
			// Travar o começo da fila também, já que o aviao
			// é adicionado la, em vez de ser no final (prioridade)
			pthread_mutex_lock(&fila->saiFila);
			fila->primeiro->anterior = elemento;
			elemento->proximo = fila->primeiro;
			elemento->anterior = NULL;
			fila->primeiro = elemento;
			fila->n_elementos++;
			pthread_mutex_unlock(&fila->saiFila);
		} else {
			// Sem prioridade de pouso, vai para o final da fila
			fila->ultimo->proximo = elemento;
			elemento->anterior = fila->ultimo;
			fila->n_elementos++;
		}
	}
	pthread_mutex_unlock(&fila->entraFila);
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
	pthread_mutex_unlock(&fila->saiFila);

	return aviao;
}