#include "aviao.h"

/**
 * aviao.c
 * Implementação das funções do aviao.h
 * Descrições em aviao.h
 **/

aviao_t * criar_aviao(size_t id) {
  aviao_t *aviao = (aviao_t *) malloc(sizeof (aviao_t));

  unsigned int seed = time(NULL);
  short combustivel = rand_r(&seed) % 100;

  pthread_t thread;
  aviao->thread = thread;
  aviao->combustivel = combustivel;
  aviao->id = id;

  return aviao;
}

void desaloca_aviao(aviao_t* aviao) {
	free(aviao);
}
