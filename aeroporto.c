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
	aeroporto->filasPousoDecolagem = (fila_ordenada_t *) malloc(npistas * sizeof(fila_ordenada_t));
	aeroporto->portoes = (pthread_mutex_t *) malloc(nportoes * sizeof(pthread_mutex_t));
	aeroporto->esteiras = (sem_t *) malloc(nesteiras * sizeof(sem_t));

	for (int i = 0 ; i < npistas; i++) {
		pthread_mutex_init((aeroporto->pistas + i), NULL);
		aeroporto->filasPouso = (fila_ordenada_t *) criar_fila();
	}

	sem_init(&aeroporto->pistasLivres, 0, npistas);

	for (int i = 0; i < nportoes; i++)
		pthread_mutex_init((aeroporto->portoes + i), NULL);

	sem_init(&aeroporto->portoesLivres,0,n_portoes);

	for (int i = 0; i < nesteiras; i++)
		sem_init((aeroporto->esteiras + i), 0, n_max_avioes_esteira);

	return aeroporto;	
}

void aproximacao_aeroporto (aeroporto_t* aeroporto, aviao_t* aviao) {
	if (aviao->combustivel < 10) {
		// Aproxima com logica de prioridade para pouso
		aproximarNaMelhorFila(aeroporto, aviao);
	} else {
		// Aviao não tem prioridade, insere na ultima posição 
		// de uma fila de pouso e decolagem de uma pista
		inserirUltimo(aeroporto->(filasPousoDecolagem + (aviao->id % aeroporto->n_pistas)), aviao, 1);
	}
}

void aproximarNaMelhorFila (aeroporto_t *aeroporto, aviao_t *aviao) {
	int cont = 1, chegouUltimo = 0, inseriu = 0;
	do {
		elemento_t *elementoAux;
		for (int i = 0; i < aeroporto->n_pistas; i++) {
			int index = 1;
			elementoAux = aeroporto->(filasPousoDecolagem + i)->primeiro;
			if (cont < aeroporto->(filasPousoDecolagem + i)->n_elementos) {
					// Vai até o elemento na posição cont em cada fila
				for (int j = 1 ; j < cont; j++) { 
					elementoAux = elementoAux->proximo;
					if (elementoAux->dado->combustivel >= 10) {
						// Insere na pos do elemento se este n tiver prioridade
						inserir(aeroporto->(filasPousoDecolagem + i), aviao, cont);
						liberaTodasFilas(aeroporto);
						// Sinalização para break do primeiro laço		
						inseriu = 1;
						// Sai do segundo laço
						break;
					}
				} else {
					if (cont == aeroporto->(filasPousoDecolagem + i)->n_elementos)
						// Sinaliza fila com todos elementos c/ prioridade
						chegouUltimo++;
				}
			}
			// sair do segundo laço de acordo com a sinalização
			if (inseriu = 1)
				break;
			if (chegouUltimo == n_pistas) {
				// Todas filas tem todos elementos c/ prioridade
				// Procura a menor fila (c/ menos elementos, p/ aproximar)
				index = acharFilaComMenosAvioes(aeroporto);
				inserirUltimo(aeroporto->(filasPousoDecolagem + index), aviao, 0);
				liberaTodasFilas(aeroporto);
				// Sai do laço principal	
				break;
			} else {
				// sinaliza que foi verif. tds as pos. cont de tds as filas
				cont++;
			}
		} while (1);
	}
}

int acharFilaComMenosAvioes(aeroporto_t *aeroporto_t) {
	int indexMenor = 0, numMenor = 0;
	for (int i = 0; i < aeroporto->n_pistas; i++) {
		if (aeroporto->(filasPousoDecolagem +i)->n_elementos < numMenor) {
			indexMenor = i;
			numMenor = aeroporto->(filasPousoDecolagem +i)->n_elemento;
		}
	}
	return indexMenor;
}

void trancaTodasFilas(aeroporto_t *aeroporto) {
	for (int i= 0; i < aeroporto->n_pistas; i++)
		pthread_mutex_lock(&aeroporto->(filasPousoDecolagem + i)->mutexFila);
}

void liberaTodasFilas(aeroporto_t *aeroporto) {
	for (int i= 0; i < aeroporto->n_pistas; i++)
		pthread_mutex_unlock(&aeroporto->(filasPousoDecolagem->mutexFila));
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
