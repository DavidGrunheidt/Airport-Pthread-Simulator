#include "aeroporto.h"

/**
 * aeroporto.c
 * Implementação das funções do aeroporto.h
 * Descrições em aeroporto.h
 **/

aeroporto_t* iniciar_aeroporto (size_t* args) {
	aeroporto_t *aeroporto = (aeroporto_t *) malloc(sizeof(aeroporto_t));

	int npistas = *(args), nportoes = *(args+1), nesteiras = *(args+2), n_max_avioes_esteira = *(args+3);
	
	aeroporto->n_pistas = npistas;
	aeroporto->n_portoes = nportoes;
	aeroporto->n_esteiras = nesteiras;
	aeroporto->n_max_avioes_esteira= n_max_avioes_esteira;
	aeroporto->t_pouso_decolagem = *(args+4);
	aeroporto->t_remover_bagagens = *(args + 5);
	aeroporto->t_inserir_bagagens = *(args + 6);
	aeroporto->t_bagagens_esteira = *(args + 7);

	aeroporto->pistas = (pthread_mutex_t *) malloc(npistas * sizeof(pthread_mutex_t));
	aeroporto->portoes = (pthread_mutex_t *) malloc(nportoes * sizeof(pthread_mutex_t));
	aeroporto->esteiras = (sem_t *) malloc(nesteiras * sizeof(sem_t));

	for (int i = 0 ; i < npistas; i++)
		pthread_mutex_init((aeroporto->pistas + i), NULL);

	for (int i = 0; i < nportoes; i++)
		pthread_mutex_init((aeroporto->portoes + i), NULL);

	for (int i = 0; i < nesteiras; i++)
		sem_init((aeroporto->esteiras + i), 0, n_max_avioes_esteira);

	pthread_mutex_init(&aeroporto->entraFila, NULL);
	pthread_mutex_init(&aeroporto->saiFila, NULL);

	aeroporto->filaPouso = (fila_ordenada_t *) criar_fila();

	return aeroporto;	
}

void aproximacao_aeroporto (aeroporto_t* aeroporto, aviao_t* aviao) {
	
}

void pousar_aviao (aeroporto_t* aeroporto, aviao_t* aviao) {

}

void acoplar_portao (aeroporto_t* aeroporto, aviao_t* aviao) {

}

void transportar_bagagens (aeroporto_t* aeroporto, aviao_t* aviao) {

}

void adicionar_bagagens_esteira (aeroporto_t* aeroporto, aviao_t* aviao) {

}

void decolar_aviao (aeroporto_t* aeroporto, aviao_t* aviao) {

}

int finalizar_aeroporto (aeroporto_t* aeroporto) {
	return 0;
}
