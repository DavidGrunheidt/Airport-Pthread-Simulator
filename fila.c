#include "fila.h"

/**
 * fila.c
 * Implementação das funções da fila.h
 * Descrições em fila.h
 **/

elemento_t * aloca_elemento (aviao_t * dado) {
	elemento_t *elemento = (elemento_t *) malloc(sizeof (elemento_t));
	return elemento;
}

void desaloca_elemento (elemento_t * elemento) {
	free(elemento->dado);
	free(elemento);
}

fila_ordenada_t * criar_fila () {
	fila_ordenada_t *fila = (fila_ordenada_t *) malloc(sizeof(fila_ordenada_t));

	fila->primeiro = NULL;
	fila->ultimo = NULL;

	fila->n_elementos = 0;

	return fila;
}

void desaloca_fila (fila_ordenada_t * fila) {
	elemento_t *elemento = fila->primeiro->proximo;
	while (elemento != fila->ultimo) {
		desaloca_elemento(elemento);
		elemento = elemento->proximo;
	}
	desaloca_elemento(elemento);

	free(fila);
}

void inserir (fila_ordenada_t * fila, aviao_t * dado) {
	elemento_t *elemento = aloca_elemento(dado);

	if (dado->combustivel < 10) {
		fila->primeiro->anterior = elemento;
		elemento->proximo = fila->primeiro;
		elemento->anterior = NULL;
		fila->primeiro = elemento;
	} else {
		fila->ultimo->proximo = elemento;
		elemento->anterior = fila->ultimo;
		elemento->proximo = NULL;
	}
	fila->n_elementos++;
}

aviao_t * remover (fila_ordenada_t * fila) {
	fila->primeiro->proximo->anterior = NULL;
	desaloca_elemento(fila->primeiro);
	fila->n_elementos--;
}
