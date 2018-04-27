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
	if (aviao->combustivel < 10) {
		// Aproxima com logica de prioridade para pouso
		 filaInserido = aproximarNaMelhorFila(aeroporto, aviao);
	} else {
		// Aviao não tem prioridade, insere na ultima posição 
		// de uma fila de pouso e decolagem de uma pista
		inserirUltimo(aeroporto->filasPousoDecolagem[filaInserido], aviao, 1);
	}
	return filaInserido;
}

size_t aproximarNaMelhorFila (aeroporto_t *aeroporto, aviao_t *aviao) {
	size_t cont = 1, chegouUltimo = 0, inseriu = 0, filaInserido = 0;
	do {
		elemento_t *elementoAux;
		for (int i = 0; i < aeroporto->n_pistas; i++) {
			elementoAux = aeroporto->filasPousoDecolagem[i]->primeiro;
			if (cont < aeroporto->filasPousoDecolagem[i]->n_elementos) {
					// Vai até o elemento na posição cont em cada fila
				for (int j = 1 ; j < cont; j++) { 
					elementoAux = elementoAux->proximo;
					if (elementoAux->dado->combustivel >= 10) {
						// Insere na pos do elemento se este n tiver prioridade
						filaInserido = i;
						inserir(aeroporto->filasPousoDecolagem[i], aviao, cont);
						liberaTodasFilas(aeroporto);
						// Sinalização para break do primeiro laço		
						inseriu = 1;
						// Sai do segundo laço
						break;
					}
				}
			} else {
				if (cont == aeroporto->filasPousoDecolagem[i]->n_elementos)
						// Sinaliza fila com todos elementos c/ prioridade
					chegouUltimo++;
			}
		}
			// sair do segundo laço de acordo com a sinalização
		if (inseriu = 1)
			break;
		if (chegouUltimo == aeroporto->n_pistas) {
			// Todas filas tem todos elementos c/ prioridade
			// Procura a menor fila (c/ menos elementos, p/ aproximar)
			filaInserido = acharFilaComMenosAvioes(aeroporto);
			inserirUltimo(aeroporto->filasPousoDecolagem[filaInserido], aviao, 0);
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

void pousar_aviao (aeroporto_t* aeroporto, size_t idAviao, size_t idFilaDeAproximacao, int tempoPouso) {
	pthread_mutex_lock((aeroporto->pistas + idFilaDeAproximacao));
	// Aviao pousou na pista IdFilaDeAproximacao
	fprintf(stderr, " Avião %lu pousou na pista %lu\n", idAviao, idFilaDeAproximacao);
	// Aguarda o tempo de pouso antes de liberar a pista
    usleep(tempoPouso * 1000);
	pthread_mutex_unlock((aeroporto->pistas + idFilaDeAproximacao));

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
