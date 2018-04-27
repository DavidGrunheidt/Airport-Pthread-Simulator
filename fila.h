#ifndef FILA_AVIAO_H
#define FILA_AVIAO_H

#include "aviao.h"

typedef struct elemento_t {
	struct elemento_t * anterior;
	struct elemento_t * proximo;
	aviao_t * dado;
} elemento_t;

// Alocacao dinamica do elemento
elemento_t * aloca_elemento (aviao_t * dado);
void desaloca_elemento (elemento_t * elemento);

// Estrutura c/ fila de avioes ordenada
typedef struct {
	elemento_t * primeiro;
	elemento_t * ultimo;
	size_t n_elementos;
	pthread_mutex_t entraFila, saiFila;
} fila_ordenada_t;

// Alocacão dinâmica da fila ordenada de aviões
fila_ordenada_t * criar_fila ();

// Desaloca a fila.
void desaloca_fila (fila_ordenada_t * fila);

// Insere dados de forma ordenada na fila
void inserir (fila_ordenada_t * fila, aviao_t * dado);

// Remove o indeximo elemento da fila (considere o 1º index = 1)
aviao_t * remover (fila_ordenada_t * fila, aviao_t *aviao);

// Retorna o index que o aviao esta
size_t at (fila_ordenada_t * fila, aviao_t *aviao);
#endif
