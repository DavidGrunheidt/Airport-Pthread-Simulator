#ifndef AVIAO_H
#define AVIAO_H

#include <pthread.h>
#include <stdlib.h>


typedef struct {
  pthread_t thread; // Uso dos recursos pelo avião é monitorado pela thread
  size_t combustivel; // Indica prioridade do avião para pousar
  size_t id; // Identificador do avião
} aviao_t;

// Alocação e desalocação dinamica na memoria.
aviao_t * criar_aviao (size_t id, size_t combustivel);
void desaloca_aviao (aviao_t* aviao);

#endif
