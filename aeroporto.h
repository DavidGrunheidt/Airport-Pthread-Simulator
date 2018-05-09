#ifndef AEROPORTO_H
#define AEROPORTO_H

#include <semaphore.h>
#include <stdio.h>
#include <unistd.h>
#include "aviao.h"
#include "fila.h"

typedef struct {
	size_t nEsteira;
	size_t tBagNaEsteira;
	sem_t *semEsteira;
} argAx;

typedef size_t tempo_t;

typedef struct {
	size_t n_pistas;
	size_t n_portoes;
	size_t n_esteiras;
	size_t n_max_avioes_esteira;
	tempo_t t_pouso_decolagem;
	tempo_t t_remover_bagagens;
	tempo_t t_bagagens_esteira;
	tempo_t t_inserir_bagagens;
	tempo_t t_aproximacao_aero;

	// Ponteiros para varias pistas e portoes
	// Cada pista e portao só pode ter um aviao associado
	pthread_mutex_t *pistas, *portoes;

	// Semaforo para indicar os portoes livres antes de
	// um aviao pegar um portao
	sem_t portoesLivres;

	// Ponteiros para varias esteiras
	// Cada esteira pode ser associado com "nMax" avioes
	sem_t *esteiras;

	// Filas para as pistas do aeroporto e semaforo para indicar
	// se alguma destas pistas esta livre para pouso
	
	fila_ordenada_t **filasPousoDecolagem;
	fila_ordenada_t **filasPortoes;
	sem_t pistasLivres;
} aeroporto_t;

size_t acharFilaComMenosAvioes(fila_ordenada_t **filas, int nFilas);
void trancaTodasFilas(fila_ordenada_t **filas, int nFilas);
void liberaTodasFilas(fila_ordenada_t **filas, int nFilas);

aeroporto_t* iniciar_aeroporto (size_t* args);

// Sinaliza a intenção de pousar ou decolagem e é inserido em uma fila
// esperando sua vez para realizar a ação
size_t solicitarPista (aeroporto_t* aeroporto, aviao_t* aviao, size_t pousoOuDecolagem);

// Verifica se tem prioridade ou não e insere na melhor fila
// ou seja, a fila com menos elementos
size_t aproximarNaMelhorFila (aeroporto_t *aeroporto, aviao_t *aviao, size_t *index);

// auxiliar da funcao aproximaNaMelhorFila (para aviões com prioridade)
void procurarFilaComAviaoSemPrioridade(aeroporto_t *aeroporto, aviao_t *aviao, size_t *filaInserido, size_t *index, size_t *chegouUltimo, size_t *inseriu, size_t cont);

// Verifica se o aviao é o primeiro da fila para usar a pista 
// daquela fila. Se for então chama sua função auxiliar para
// pousar ou decolar e utilizar o recurso pista
void usarPistaSePrimeiroDaFilaPista(aeroporto_t *aeroporto, aviao_t *aviao, size_t idFilaDeAproximacao, size_t pousoOuDecolagem);

// Funcao onde a utilização do recurso pista é feita
// para pouso ou decolagem
void usarPista (aeroporto_t* aeroporto, size_t idAviao, size_t idPista, size_t pousoOuDecolagem);

// Avião é primeiramente inserido em uma fila sinalizando a intenção
// de querer acoplar. Assim que ele pousa ele é inserido na fila e se quando
// este for o primeiro da fila entao ele acopla. É importante ter uma fila
// caso o numero de portoes seja pouco, já que nem sempre terá
// portoes para todos que pousaram.
size_t solicitarAcoplagem(aeroporto_t *aeroporto, aviao_t *aviao);

// Vereficia se é o primeiro da fila de acoplagem, se for, acopla
void acoplarSePrimeiroDafilaDeAcoplagem(aeroporto_t *aeroporto, aviao_t *aviao , size_t idFilaDeAclopagem);
// Auxiliar da função que verifica, a acoplagem é feita aqui
// realiza o consumo do portao
void acoplar_portao (aeroporto_t* aeroporto, size_t idAviao, size_t idPortao);

// Aviao aguarda uma esteira livre para transportar suas bagagens
// e depois insere novas bagagens nele. Realiza o consumo de alguma
// esteira
void transportar_bagagens (aeroporto_t* aeroporto, aviao_t* aviao, size_t idPortao);

// Aviao nao depende do tBagagensNaEsteira, logo é criada
// outra thread para esperar este tempo enquanto o aviao
// continua sua simulação, e qnd o tempo acabar
// a thread libera a esteira que o aviao anteriormente consumiu
void* simularBagagensEsteira (void *args);

// liberar o portao para outros avioes acloparem
void desaclopar_aviao (aeroporto_t* aeroporto, aviao_t* aviao, size_t idPortao);

size_t solicitarDecolagem(aeroporto_t* aeroporto, aviao_t* aviao);

// desalocação de todos os elementos do aeroporto
int finalizar_aeroporto (aeroporto_t* aeroporto);

#endif
