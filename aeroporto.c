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
	aeroporto->t_aproximacao_aero = *(args + 4);
	aeroporto->t_pouso_decolagem = *(args+5);
	aeroporto->t_remover_bagagens = *(args + 6);
	aeroporto->t_inserir_bagagens = *(args + 7);
	aeroporto->t_bagagens_esteira = *(args + 8);

	aeroporto->pistas = (pthread_mutex_t *) malloc(npistas * sizeof(pthread_mutex_t));
	aeroporto->filasPousoDecolagem = (fila_ordenada_t **) malloc(npistas * sizeof(fila_ordenada_t *));
	aeroporto->portoes = (pthread_mutex_t *) malloc(nportoes * sizeof(pthread_mutex_t));
	aeroporto->esteiras = (sem_t *) malloc(nesteiras * sizeof(sem_t));

	for (int i = 0 ; i < npistas; i++) {
		pthread_mutex_init((aeroporto->pistas + i), NULL);
		aeroporto->filasPousoDecolagem[i] = criar_fila();
	}

	sem_init(&aeroporto->pistasLivres, 0, npistas);

	for (int i = 0; i < nportoes; i++)
		pthread_mutex_init((aeroporto->portoes + i), NULL);

	sem_init(&aeroporto->portoesLivres,0,nportoes);

	for (int i = 0; i < nesteiras; i++)
		sem_init((aeroporto->esteiras + i), 0, n_max_avioes_esteira);

	return aeroporto;	
}

size_t aproximacao_aeroporto (aeroporto_t* aeroporto, aviao_t* aviao) {
	size_t filaInserido = aviao->id % aeroporto->n_pistas;
	size_t index;
	if (aviao->combustivel < 10) {
		// Verifica se a fila tem algum elemento
		pthread_mutex_lock(&aeroporto->filasPousoDecolagem[filaInserido]->mutexFila);
		// Verifica se o aviao é o primeiro elemento da fila
		if (aeroporto->filasPousoDecolagem[filaInserido]->n_elementos == 0) {
			index = 0;
			elemento_t *elemento = aloca_elemento(aviao);
			inserirPrimeiro(aeroporto->filasPousoDecolagem[filaInserido], elemento);
			pthread_mutex_unlock(&aeroporto->filasPousoDecolagem[filaInserido]->mutexFila);
		} else {
			pthread_mutex_unlock(&aeroporto->filasPousoDecolagem[filaInserido]->mutexFila);
			// Aproxima com logica de prioridade para pouso
			filaInserido = aproximarNaMelhorFila(aeroporto, aviao, &index);
		}
	} else {
		// Aviao não tem prioridade, insere na ultima posição 
		// de uma fila de pouso e decolagem de uma pista
		index = inserirUltimo(aeroporto->filasPousoDecolagem[filaInserido], aviao, 1);
	}

	printf("Avião (%lu) com (%lu)%% de combustivel aproximando para pouso na pista (%lu), posição na fila = (%lu) \n", aviao->id, aviao->combustivel, filaInserido, index);	
	fflush(stdout);

	// Tempo desde a requisição até ele chegar no aeroporto
	// (TEMPO DE APROXIMAÇÃO), sem isso todas as filas
	// sempre terão somente 1 aviao (muito rapido)
	usleep(aeroporto->t_aproximacao_aero);

	return filaInserido;
}

size_t aproximarNaMelhorFila (aeroporto_t *aeroporto, aviao_t *aviao, size_t *index) {
	size_t cont = 0, chegouUltimo = 0, inseriu = 0, filaInserido = 0;
	trancaTodasFilas(aeroporto);
	do {
		elemento_t *elementoAux;
		for (int i = 0; i < aeroporto->n_pistas; i++) {
			elementoAux = aeroporto->filasPousoDecolagem[i]->primeiro;
			if (cont < aeroporto->filasPousoDecolagem[i]->n_elementos) {
					// Vai até o elemento na posição cont em cada fila
				for (int j = 1 ; j < cont; j++) 
					elementoAux = elementoAux->proximo;
				if (elementoAux->dado->combustivel >= 10) {
					// Insere na pos do elemento se este n tiver prioridade
					filaInserido = i;
					inserir(aeroporto->filasPousoDecolagem[i], aviao, cont+1);
					*index = cont;
					liberaTodasFilas(aeroporto);
					// Sinalização para break do primeiro laço		
					inseriu = 1;
					// Sai do segundo laço
					break;
				}
			} else {
				if (cont == aeroporto->filasPousoDecolagem[i]->n_elementos)
						// Sinaliza fila com todos elementos c/ prioridade
					chegouUltimo++;
			}
		}
		// sair do segundo laço de acordo com a sinalização
		if (inseriu == 1) 
			break;
		if (chegouUltimo == aeroporto->n_pistas) {
			// Todas filas tem todos elementos c/ prioridade
			// Procura a menor fila (c/ menos elementos, p/ aproximar)
			printf("Cheguei\n");
			fflush(stdout);
			filaInserido = acharFilaComMenosAvioes(aeroporto);
			*index = inserirUltimo(aeroporto->filasPousoDecolagem[filaInserido], aviao, 0)-1;
			liberaTodasFilas(aeroporto);
				// Sai do laço principal	
			break;
		} else {
				// sinaliza que foi verif. tds as pos. cont de tds as filas
			cont++;
		}
	} while (1);
	return filaInserido;
}

size_t acharFilaComMenosAvioes(aeroporto_t *aeroporto) {
	int indexMenor = 0, numMenor = 0;
	for (int i = 0; i < aeroporto->n_pistas; i++) {
		if (aeroporto->filasPousoDecolagem[i]->n_elementos < numMenor) {
			indexMenor = i;
			numMenor = aeroporto->filasPousoDecolagem[i]->n_elementos;
		}
	}
	return indexMenor;
}

void trancaTodasFilas(aeroporto_t *aeroporto) {
	for (int i= 0; i < aeroporto->n_pistas; i++)
		pthread_mutex_lock(&aeroporto->filasPousoDecolagem[i]->mutexFila);
}

void liberaTodasFilas(aeroporto_t *aeroporto) {
	for (int i= 0; i < aeroporto->n_pistas; i++)
		pthread_mutex_unlock(&aeroporto->filasPousoDecolagem[i]->mutexFila);
}

void pousar_aviao (aeroporto_t* aeroporto, size_t idAviao, size_t idFilaDeAproximacao) {
	pthread_mutex_lock((aeroporto->pistas + idFilaDeAproximacao));
	// Aviao pousou na pista IdFilaDeAproximacao
	usleep(aeroporto->t_pouso_decolagem);
	printf("Avião (%lu) pousou na pista %lu\n", idAviao, idFilaDeAproximacao);
	fflush(stdout);
	remover(aeroporto->filasPousoDecolagem[idFilaDeAproximacao]);
	// Aguarda o tempo de pouso antes de liberar a pista
	pthread_mutex_unlock((aeroporto->pistas + idFilaDeAproximacao));
	sem_post(&aeroporto->pistasLivres);
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
	for (int i = 0 ; i < aeroporto->n_pistas; i++) {
		pthread_mutex_destroy(aeroporto->pistas + i);
		desaloca_fila(aeroporto->filasPousoDecolagem[i]);
	}

	sem_destroy(&aeroporto->pistasLivres);

	for (int i = 0; i < aeroporto->n_portoes; i++)
		pthread_mutex_destroy(aeroporto->portoes + i);

	sem_destroy(&aeroporto->portoesLivres);

	for (int i = 0; i < aeroporto->n_esteiras; i++)
		sem_destroy(aeroporto->esteiras + i);
}
