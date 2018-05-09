#include "aeroporto.h"

/**
 * aeroporto.c
 * Implementação das funções do aeroporto.h
 * Descrições em aeroporto.h
 **/

aeroporto_t* iniciar_aeroporto (size_t *args) {
	aeroporto_t *aeroporto = (aeroporto_t *) malloc(sizeof(aeroporto_t));

	int npistas = *(args), nportoes = *(args+1), nesteiras = *(args+2), n_max_avioes_esteira = *(args+3);
	
	aeroporto->n_pistas = npistas;
	aeroporto->n_portoes = nportoes;
	aeroporto->n_esteiras = nesteiras;
	aeroporto->n_max_avioes_esteira= n_max_avioes_esteira;
	aeroporto->t_aproximacao_aero = *(args + 4);
	aeroporto->t_pouso_decolagem = *(args+5);
	aeroporto->t_remover_bagagens = *(args + 6);
	aeroporto->t_bagagens_esteira = *(args + 7);
	aeroporto->t_inserir_bagagens = *(args + 8);

	aeroporto->pistas = (pthread_mutex_t *) malloc(npistas * sizeof(pthread_mutex_t));
	aeroporto->filasPousoDecolagem = (fila_ordenada_t **) malloc(npistas * sizeof(fila_ordenada_t *));
	aeroporto->portoes = (pthread_mutex_t *) malloc(nportoes * sizeof(pthread_mutex_t));
	aeroporto->filasPortoes = (fila_ordenada_t **) malloc (nportoes * sizeof(fila_ordenada_t *));
	aeroporto->esteiras = (sem_t *) malloc(nesteiras * sizeof(sem_t));

	for (int i = 0 ; i < npistas; i++) {
		pthread_mutex_init((aeroporto->pistas + i), NULL);
		aeroporto->filasPousoDecolagem[i] = criar_fila();
	}

	sem_init(&aeroporto->pistasLivres, 0, npistas);

	for (int i = 0; i < nportoes; i++) {
		pthread_mutex_init((aeroporto->portoes + i), NULL);
		aeroporto->filasPortoes[i] = criar_fila();
	}

	sem_init(&aeroporto->portoesLivres,0,nportoes);

	for (int i = 0; i < nesteiras; i++)
		sem_init((aeroporto->esteiras + i), 0, n_max_avioes_esteira);

	return aeroporto;	
}

size_t solicitarPista (aeroporto_t* aeroporto, aviao_t* aviao, size_t pousoOuDecolagem) {
	size_t filaInserido = aviao->id % aeroporto->n_pistas;
	size_t index;
	if (aviao->combustivel < 10) {
		// Verifica se a fila tem algum elemento
		pthread_mutex_lock(&aeroporto->filasPousoDecolagem[filaInserido]->mutexFila);
		// Verifica se o aviao é o primeiro elemento da fila
		if (aeroporto->filasPousoDecolagem[filaInserido]->n_elementos == 0) {
			index = 0;
			inserirPrimeiro(aeroporto->filasPousoDecolagem[filaInserido], aloca_elemento(aviao));
			pthread_mutex_unlock(&aeroporto->filasPousoDecolagem[filaInserido]->mutexFila);
		} else {
			pthread_mutex_unlock(&aeroporto->filasPousoDecolagem[filaInserido]->mutexFila);
			// Aproxima com logica de prioridade para pouso
			filaInserido = aproximarNaMelhorFila(aeroporto, aviao, &index);
		}
	} else {
		// Insere o aviao na fila com menor numero de avioes
		// esperando para pousar
		trancaTodasFilas(aeroporto->filasPousoDecolagem, aeroporto->n_pistas);
		filaInserido = acharFilaComMenosAvioes(aeroporto->filasPousoDecolagem, aeroporto->n_pistas);
		index = inserirUltimo(aeroporto->filasPousoDecolagem[filaInserido], aviao, 0);
		liberaTodasFilas(aeroporto->filasPousoDecolagem, aeroporto->n_pistas);
	}

	if (pousoOuDecolagem == 0) {
		printf("Avião (%lu) com (%lu)%% de combustivel aproximando para pouso na pista (%lu), posição na fila = (%lu) \n", aviao->id, aviao->combustivel, filaInserido, index);	
		fflush(stdout);
	} else {
		printf("Aviao (%lu) entrou na fila da pista (%lu) para decolar, posição = (%lu)\n", aviao->id, filaInserido, index);
	}

	// Tempo desde a requisição até ele chegar no aeroporto
	usleep(aeroporto->t_aproximacao_aero);

	return filaInserido;
}

size_t aproximarNaMelhorFila (aeroporto_t *aeroporto, aviao_t *aviao, size_t *index) {
	size_t cont = 0, chegouUltimo = 0, inseriu = 0, filaInserido = 0;
	trancaTodasFilas(aeroporto->filasPousoDecolagem, aeroporto->n_pistas);
	do {
		procurarFilaComAviaoSemPrioridade(aeroporto, aviao, &filaInserido, index, &chegouUltimo, &inseriu, cont);
		// sair do segundo laço de acordo com a sinalização
		if (inseriu == 1) 
			break;
		if (chegouUltimo == aeroporto->n_pistas) {
			// Todas filas tem todos elementos c/ prioridade
			// Procura a menor fila (c/ menos elementos, p/ aproximar)
			filaInserido = acharFilaComMenosAvioes(aeroporto->filasPousoDecolagem, aeroporto->n_pistas);
			*index = inserirUltimo(aeroporto->filasPousoDecolagem[filaInserido], aviao, 0);
			liberaTodasFilas(aeroporto->filasPousoDecolagem, aeroporto->n_pistas);
				// Sai do laço principal	
			break;
		} else {
				// sinaliza que foi verif. tds as pos. cont de tds as filas
			cont++;
		}
	} while (1);
	return filaInserido;
}

void procurarFilaComAviaoSemPrioridade(aeroporto_t *aeroporto, aviao_t *aviao, size_t *filaInserido, size_t *index, size_t *chegouUltimo, size_t *inseriu, size_t cont) {
	elemento_t *elementoAux;
	for (int i = 0; i < aeroporto->n_pistas; i++) {
		elementoAux = aeroporto->filasPousoDecolagem[i]->primeiro;
		if (cont < aeroporto->filasPousoDecolagem[i]->n_elementos) {
					// Vai até o elemento na posição cont em cada fila
			for (int j = 0 ; j < cont; j++) 
				elementoAux = elementoAux->proximo;
			if (elementoAux->dado->combustivel >= 10) {
					// Insere na pos do elemento se este n tiver prioridade
				*filaInserido = i;
				inserir(aeroporto->filasPousoDecolagem[i], aviao, cont);
				*index = cont;
				liberaTodasFilas(aeroporto->filasPousoDecolagem, aeroporto->n_pistas);
				// Sinalização para break do laço da funcao 
				// aproximarNaMelhorFila		
				*inseriu = 1;
				// Finaliza esta funcao
				break;
			}
		} else {
			if (cont == aeroporto->filasPousoDecolagem[i]->n_elementos)
			// Sinaliza fila com todos elementos c/ prioridade
				*chegouUltimo++;
		}
	}
}

size_t acharFilaComMenosAvioes(fila_ordenada_t **filas, int nFilas) {
	int indexMenor = 0, numMenor = filas[0]->n_elementos;
	for (int i = 1; i < nFilas; i++) {
		if (filas[i]->n_elementos < numMenor) {
			indexMenor = i;
			numMenor = filas[i]->n_elementos;
		}
	}
	return indexMenor;
}

void trancaTodasFilas(fila_ordenada_t **filas, int nFilas) {
	for (int i= 0; i < nFilas; i++)
		pthread_mutex_lock(&filas[i]->mutexFila);
}

void liberaTodasFilas(fila_ordenada_t **filas, int nFilas) {
	for (int i= 0; i < nFilas; i++)
		pthread_mutex_unlock(&filas[i]->mutexFila);
}

void usarPistaSePrimeiroDaFilaPista(aeroporto_t *aeroporto, aviao_t *aviao, size_t idFilaPista, size_t pousoOuDecolagem) {
	while (1) {
        // Aguarda uma pista das npistas livres
		sem_wait(&aeroporto->pistasLivres);
        // Verifica se ele é o primeiro da fila
        // Trava o acesso a fila para verificação e talvez utilização
        // Obs: unica logica de lock e unlock fora da fila (Otimização)
		pthread_mutex_lock(&aeroporto->filasPousoDecolagem[idFilaPista]->mutexFila);
		if (aeroporto->filasPousoDecolagem[idFilaPista]->primeiro->dado->id == aviao->id) {
            // Se for o primeiro então pousa o aviao
            // Obs: mutex da fila é destravado dentro da func de remoção
            // na logica da fila
			usarPista(aeroporto, aviao->id, idFilaPista, pousoOuDecolagem);
			break;
		} else {
            // Se não for o primeiro então precisa destravar o mutex já que
            // a parte de destravamento que esta dentro da func de remoção do
            // aviao da fila de aproximação (remover) não acontecerá.
			pthread_mutex_unlock(&aeroporto->filasPousoDecolagem[idFilaPista]->mutexFila);
			sem_post(&aeroporto->pistasLivres);
		}
	}
}

void usarPista (aeroporto_t* aeroporto, size_t idAviao, size_t idPista, size_t pousoOuDecolagem) {
	pthread_mutex_lock((aeroporto->pistas + idPista));
	// Aviao usou a pista IdFilaDeAproximacao p/ pouso ou decolagem
	usleep(aeroporto->t_pouso_decolagem);
	remover(aeroporto->filasPousoDecolagem[idPista]);
	// Aguarda o tempo de pouso antes de liberar a pista
	pthread_mutex_unlock((aeroporto->pistas + idPista));

	if (pousoOuDecolagem == 0) {
		printf("Avião (%lu) pousou na pista %lu\n", idAviao, idPista);
		fflush(stdout);
	} else {
		printf("Avião (%lu) decolou na pista %lu\n", idAviao, idPista);
		fflush(stdout);
	}

	sem_post(&aeroporto->pistasLivres);
}

size_t solicitarAcoplagem(aeroporto_t *aeroporto, aviao_t *aviao) {
	size_t filaInserido = aviao->id % aeroporto->n_portoes;
	size_t index = 0;

	// Trava o mutex para verificar a fila
	pthread_mutex_lock(&aeroporto->filasPortoes[filaInserido]->mutexFila);
	if (aeroporto->filasPortoes[filaInserido]->n_elementos == 0) {
		inserirPrimeiro(aeroporto->filasPortoes[filaInserido], aloca_elemento(aviao));
		pthread_mutex_unlock(&aeroporto->filasPortoes[filaInserido]->mutexFila);
	} else {
		pthread_mutex_unlock(&aeroporto->filasPortoes[filaInserido]->mutexFila);
		trancaTodasFilas(aeroporto->filasPortoes, aeroporto->n_portoes);
		filaInserido = acharFilaComMenosAvioes(aeroporto->filasPortoes, aeroporto->n_portoes);
		index = inserirUltimo(aeroporto->filasPortoes[filaInserido], aviao, 0);
		liberaTodasFilas(aeroporto->filasPortoes, aeroporto->n_portoes);
	}
	printf("Aviao (%lu) requisitou acoplagem no portão (%lu), posição = (%lu) \n", aviao->id, filaInserido, index);

	return filaInserido;
}

void acoplarSePrimeiroDafilaDeAcoplagem(aeroporto_t *aeroporto,aviao_t *aviao, size_t idFilaDeAcoplagem) {
	while (1) {
        // Aguarda um portao dos nportoes livres
		sem_wait(&aeroporto->portoesLivres);
        // Verifica se ele é o primeiro da fila
        // Trava o acesso a fila para verificação e talvez utilização
        // Obs: unica logica de lock e unlock fora da fila (Otimização)
		pthread_mutex_lock(&aeroporto->filasPortoes[idFilaDeAcoplagem]->mutexFila);
		if (aeroporto->filasPortoes[idFilaDeAcoplagem]->primeiro->dado->id == aviao->id) {
            // Se for o primeiro então acopla o aviao
            // Obs: mutex da fila é destravado dentro da func de remoção
            // na logica da fila 
			acoplar_portao(aeroporto, aviao->id, idFilaDeAcoplagem);
			break;
		} else {
            // Se não for o primeiro então precisa destravar o mutex já que
            // a parte de destravamento que esta dentro da func de remoção do
            // aviao da fila de acoplação (remover) não acontecerá.
			pthread_mutex_unlock(&aeroporto->filasPortoes[idFilaDeAcoplagem]->mutexFila);
			sem_post(&aeroporto->portoesLivres);
		}
	}
}

void acoplar_portao (aeroporto_t* aeroporto, size_t idAviao, size_t idPortao) {
	pthread_mutex_lock((aeroporto->portoes + idPortao));
	// Aviao pousou na pista IdFilaDeAproximacao
	remover(aeroporto->filasPortoes[idPortao]);
	// Aguarda o tempo de pouso antes de liberar a pista

	printf("Avião (%lu) acoplado  no portao (%lu)\n", idAviao, idPortao);
	fflush(stdout);
}

void transportar_bagagens (aeroporto_t* aeroporto, aviao_t* aviao, size_t idPortao) {
	size_t idEsteira = idPortao % aeroporto->n_esteiras;
	// Espera uma esteira livre para poder remover as bagagens do aviao
	sem_wait(&aeroporto->esteiras[idEsteira]);
	// Conseguiu achar uma esteira livre então espera
	// o tempo de remover as bagagens
	usleep(aeroporto->t_remover_bagagens);

	printf("Aviao (%lu) removeu suas bagagens e colocou na esteira (%lu)\n", aviao->id, idEsteira);
	fflush(stdout);

	// Argumentos para thread que espera as bagagens para 
	// liberar a esteira
	argAx *args = (argAx *) malloc (sizeof(argAx));
	args->nEsteira = idEsteira;
	args->tBagNaEsteira = aeroporto->t_bagagens_esteira;
	args->semEsteira = &aeroporto->esteiras[idEsteira];

	pthread_t bagagensEsteira;
	pthread_create(&bagagensEsteira, NULL, simularBagagensEsteira, (void *) args);
	//pthread_detach(bagagensEsteira);

	// Aguarda o tempo de inserir as bagagens no aviao
	usleep(aeroporto->t_inserir_bagagens);
	
	printf("Avião (%lu) colocou as bagagens do proximo voo e está pronto para desacoplar\n", aviao->id);
}

void* simularBagagensEsteira (void *args) {
	argAx *arg = ((argAx *)args);

	size_t nEsteira = arg->nEsteira;
	size_t tBagNaEsteira = arg->tBagNaEsteira;
	sem_t *semEsteira = arg->semEsteira;

	free(arg);

	usleep(tBagNaEsteira);

	sem_post(semEsteira);

	printf("Esteira (%lu) liberou um lugar para um avião colocar suas bagagens\n", nEsteira);
	fflush(stdout);

	pthread_exit(NULL);
}

void desaclopar_aviao (aeroporto_t* aeroporto, aviao_t* aviao, size_t idPortao) {
	pthread_mutex_unlock((aeroporto->portoes + idPortao));
	sem_post(&aeroporto->portoesLivres);
	printf("Aviao (%lu) desacoplou do portao (%lu) e está pronto para decolar\n",aviao->id, idPortao);
}

size_t solicitarDecolagem(aeroporto_t* aeroporto, aviao_t* aviao) {
	trancaTodasFilas(aeroporto->filasPousoDecolagem, aeroporto->n_pistas);
	size_t idFilaDeDecolagem = acharFilaComMenosAvioes(aeroporto->filasPousoDecolagem, aeroporto->n_pistas);
	size_t index = inserirUltimo(aeroporto->filasPousoDecolagem[idFilaDeDecolagem], aviao, 0);
	liberaTodasFilas(aeroporto->filasPousoDecolagem, aeroporto->n_pistas);

	printf("Aviao (%lu) entrou na fila da pista (%lu) para decolar, posição = (%lu)\n", aviao->id, idFilaDeDecolagem, index);

	return idFilaDeDecolagem;
}

int finalizar_aeroporto (aeroporto_t* aeroporto) {
	for (int i = 0 ; i < aeroporto->n_pistas; i++) {
		pthread_mutex_destroy(aeroporto->pistas + i);
		desaloca_fila(aeroporto->filasPousoDecolagem[i]);
	}

	free(aeroporto->pistas);
	sem_destroy(&aeroporto->pistasLivres);
	free(aeroporto->filasPousoDecolagem);

	for (int i = 0; i < aeroporto->n_portoes; i++) {
		pthread_mutex_destroy(aeroporto->portoes + i);
		desaloca_fila(aeroporto->filasPortoes[i]);
	}

	free(aeroporto->portoes);
	sem_destroy(&aeroporto->portoesLivres);
	free(aeroporto->filasPortoes);

	for (int i = 0; i < aeroporto->n_esteiras; i++)
		sem_destroy(aeroporto->esteiras + i);

	free(aeroporto->esteiras);
}
