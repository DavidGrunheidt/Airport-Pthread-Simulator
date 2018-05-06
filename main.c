

#include "aeroporto.h"
#include "fila.h"

#define NOVO_AVIAO_MIN 30000 // 30 milisegundos
#define NOVO_AVIAO_MAX 120000 // 120  milisegundos
#define COMBUSTIVEL_MIN 5
#define COMBUSTIVEL_MAX 15
#define TEMPO_APROXIMACAO_AERO  700000 //  700 milisegundos
#define TEMPO_POUSO_DECOLAGEM 40
#define TEMPO_REMOVER_BAGAGENS 90
#define TEMPO_INSERIR_BAGAGENS 110
#define TEMPO_BAGAGENS_ESTEIRA 200
#define TEMPO_SIMULACAO 10000 // 10 segundos

void criarAviao(unsigned int seed, size_t idAviao, aeroporto_t *aero);
void *simularAviao(void *arg);
void pousar(aeroporto_t *meu_aeroporto, aviao_t *aviao, size_t idFilaDeAproximacao);

typedef struct {
    aeroporto_t *aeroporto;
    aviao_t *aviao;
} arg_t;

// Variáveis temporais (inicio t_)
size_t t_novo_aviao_min, t_novo_aviao_max;
size_t t_aproximacao_aero, t_pouso_decolagem;
size_t t_remover_bagagens, t_inserir_bagagens;
size_t t_bagagens_esteira, t_simulacao;

// Variáveis discretas (inicio n_)
size_t n_pistas, n_portoes;
size_t n_max_avioes_esteira, n_esteiras;
size_t n_args;

// Variáveis de prioridade (inicio p_)
size_t p_combustivel_min, p_combustivel_max;

int main (int argc, char** argv) {
    if (argc == 5) { // Argumentos sem tempos de execução
        t_novo_aviao_min = NOVO_AVIAO_MIN;
        t_novo_aviao_max = NOVO_AVIAO_MAX;
        t_aproximacao_aero = TEMPO_APROXIMACAO_AERO;
        t_pouso_decolagem = TEMPO_POUSO_DECOLAGEM;
        t_remover_bagagens = TEMPO_REMOVER_BAGAGENS;
        t_inserir_bagagens = TEMPO_INSERIR_BAGAGENS;
        t_bagagens_esteira = TEMPO_BAGAGENS_ESTEIRA;
        t_simulacao = TEMPO_SIMULACAO;
        p_combustivel_min = COMBUSTIVEL_MIN;
        p_combustivel_max = COMBUSTIVEL_MAX;
        n_pistas = atoi(argv[1]);
        n_portoes = atoi(argv[2]);
        n_max_avioes_esteira = atoi(argv[3]);
        n_esteiras = atoi(argv[4]);

    } else if (argc == 15) { // Argumentos com tempos de execução
        int i = 0; // Este contador será incrementado antes de coletar um argumento
        t_novo_aviao_min = atoi(argv[++i]);
        t_novo_aviao_max = atoi(argv[++i]);
        p_combustivel_min = atoi(argv[++i]);
        p_combustivel_max = atoi(argv[++i]);
        t_aproximacao_aero = atoi(argv[++i]);
        t_pouso_decolagem = atoi(argv[++i]);
        n_pistas = atoi(argv[++i]);
        t_remover_bagagens = atoi(argv[++i]);
        t_inserir_bagagens = atoi(argv[++i]);
        n_portoes = atoi(argv[++i]);
        n_max_avioes_esteira = atoi(argv[++i]);
        t_bagagens_esteira = atoi(argv[++i]);
        n_esteiras = atoi(argv[++i]);
        t_simulacao = atoi(argv[++i]);

    } else { // Número incorreto de argumentos
        printf("Todas as entradas são inteiros positivos!!\nUso:\n");
        printf("./aeroporto  NOVO_AVIAO_MIN  NOVO_AVIAO_MAX\n");
        printf("COMBUSTIVEL_MIN COMBUSTIVEL_MAX   TEMPO_APROXIMACAO_AERO\n");
        printf("TEMPO_POUSO_DECOLAGEM  NUMERO_PISTAS  TEMPO_REMOVER_BAGAGENS\n");
        printf("TEMPO_INSERIR_BAGAGENS  NUMERO_PORTOES  MAXIMO_AVIOES_ESTEIRA\n");
        printf("TEMPO_BAGAGENS_ESTEIRA  NUMERO_ESTEIRAS  TEMPO_SIMULACAO\n");
        printf("----------OU----------\n");
        printf("./airport  NUMERO_PISTAS  NUMERO_PORTOES  MAXIMO_AVIOES_ESTEIRA  NUMERO_ESTEIRAS\n");
        return 0;
    }

    // Impressão com os parâmetros selecionados para simulação
    printf("Simulação iniciada com tempo total: %lu\n", t_simulacao);
    printf("Tempo para criação de aviões: %lu - %lu\n", t_novo_aviao_min, t_novo_aviao_max);
    printf("Número de pistas de pouso: %lu\n", n_pistas);
    printf("Tempo de pouso e decolagem: %lu\n", t_pouso_decolagem);
    printf("Número de portões de embarque: %lu\n", n_portoes);
    printf("Tempo de inserção (%lu) e remoção (%lu) de bagagens\n", t_inserir_bagagens, t_remover_bagagens);
    printf("Número de esteiras: %lu, com %lu aviões por esteira\n", n_esteiras, n_max_avioes_esteira);
    printf("Tempo das bagagens nas esteiras: %lu\n", t_bagagens_esteira);

    // Inicialização do aeroporto
    size_t args[9] = {n_pistas, n_portoes, n_esteiras, n_max_avioes_esteira, t_aproximacao_aero, t_pouso_decolagem, t_remover_bagagens, t_inserir_bagagens, t_bagagens_esteira};

    aeroporto_t* aero = iniciar_aeroporto(args);

    size_t contAvioes = 0;
    int wait;
    int nAvioes = 8;
    aviao_t **aviao = (aviao_t **) malloc (nAvioes * sizeof(aviao_t*));

    while (contAvioes < nAvioes) {

        // Define uma porcentagem de combustivel de 5 a 80%
        size_t combustivel = p_combustivel_min + rand() % (p_combustivel_max - p_combustivel_min);

        // Aloca memoria dinamica na heap para struct aviao
        aviao[contAvioes] = criar_aviao(contAvioes, combustivel);

        // Aloca dinamicamente os argumentos da função, de modo que 
        // eles não sejam destruidos no final do laço
        // thread é responsavel por desalocar (free)
        arg_t *argAux = (arg_t *) malloc(sizeof(arg_t));
        argAux->aeroporto = aero;
        argAux->aviao = aviao[contAvioes];

        // Cria a thread que representa o aviao
        // Todo a logica principal do programa se passa aqui
        pthread_create(&aviao[contAvioes]->thread, NULL, simularAviao,(void *) argAux);

        wait = t_novo_aviao_min + rand() % (t_novo_aviao_max - t_novo_aviao_min); 
        // Esperar um tempo para criar outro aviao
        usleep(wait);
        contAvioes++;
    }  

    for (int i = 0; i < nAvioes; i++) {
        pthread_join(aviao[i]->thread, NULL);
        desaloca_aviao(aviao[i]);
    }

    free(aviao);

    finalizar_aeroporto(aero);
    return 1;
}

void *simularAviao(void *arg) {
        // args em forma de vetor para porder liberar memoria
    arg_t *args = ((arg_t *)arg);
    aeroporto_t *meu_aeroporto =  args->aeroporto;
    aviao_t *aviao = args->aviao;
        // Liberação de memoria de args
    free(args);

    size_t idFilaDeAproximacao = aproximacao_aeroporto(meu_aeroporto, aviao);
    pousar(meu_aeroporto, aviao, idFilaDeAproximacao);
}

void pousar(aeroporto_t *meu_aeroporto, aviao_t *aviao, size_t idFilaDeAproximacao) {
    while (1) {
        // Aguarda uma pista das npistas livres
        sem_wait(&meu_aeroporto->pistasLivres);
        // Verifica se ele é o primeiro da fila
        // Trava o acesso a fila para verificação e talvez utilização
        // Obs: unica logica de lock e unlock fora da fila (Otimização)
        pthread_mutex_lock(&meu_aeroporto->filasPousoDecolagem[idFilaDeAproximacao]->mutexFila);
        if (meu_aeroporto->filasPousoDecolagem[idFilaDeAproximacao]->primeiro->dado->id == aviao->id) {
            // Se for o primeiro então pousa o aviao
            // Obs: mutex da fila é destravado dentro da func de remoção
            // na logica da fila 
            pousar_aviao(meu_aeroporto, aviao->id, idFilaDeAproximacao);
            break;
        } else {
            // Se não for o primeiro então precisa destravar o mutex já que
            // a parte de destravamento que esta dentro da func de remoção do
            // aviao da fila de aproximação (remover) não acontecerá.
            pthread_mutex_unlock(&meu_aeroporto->filasPousoDecolagem[idFilaDeAproximacao]->mutexFila);
            sem_post(&meu_aeroporto->pistasLivres);
        }
    }
}

