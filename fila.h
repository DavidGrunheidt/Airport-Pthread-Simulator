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
	// Entra fila controla o final da fila
	// Sai fila controla o começo da fila
	pthread_mutex_t mutexFila;
} fila_ordenada_t;

// Alocacão dinâmica da fila ordenada de aviões
fila_ordenada_t * criar_fila ();

// Desaloca a fila.
void desaloca_fila (fila_ordenada_t * fila);

// Insere dados no indice index na fila
// Considerando que o primeiro indice = 1
// Esta funcao tem o mutex da fila controlado por quem a chama
// Visto que quem a chama verifica em qual posicao colocar antes de inserir
// e portanto para verificar é preciso também travar a fila, logo
// o travamento é feito só uma vez
void inserir (fila_ordenada_t * fila, aviao_t * dado, size_t index);

// Insere dados na ultima posição da fila
// Só deve ser utilizada pela funcao inserir, pois o mutex nao é trancado 
// neste escopo e sim na funcao que a chamou (Contexto de prioridades)
void inserirPrimeiro(fila_ordenada_t *fila, elemento_t *elemento);

// Insere dados na primeira posição da fila. Nesta função o controle
// dos mutextes pode ser feito dentro de seu escopo, caso lock == 1.
// Caso contrario o controle é feito na funcao que a chama
void inserirUltimo(fila_ordenada_t *fila, aviao_t *dado, int lock);

// Remove o primeiro elemento da fila)
aviao_t * remover (fila_ordenada_t * fila);

#endif
