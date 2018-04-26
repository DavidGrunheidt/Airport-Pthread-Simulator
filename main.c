#include <stdio.h>
#include <unistd.h>

#include "aeroporto.h"
#include "fila.h"

#define NOVO_AVIAO_MIN 30
#define NOVO_AVIAO_MAX 120
#define COMBUSTIVEL_MIN 5
#define COMBUSTIVEL_MAX 80
#define TEMPO_POUSO_DECOLAGEM 40
#define TEMPO_REMOVER_BAGAGENS 90
#define TEMPO_INSERIR_BAGAGENS 110
#define TEMPO_BAGAGENS_ESTEIRA 200
#define TEMPO_SIMULACAO 10000 // 10 segundos


void criaAviao(aviao_t *aviao, size_t contAvioes);
void *simularAviao(void *arg);

typedef struct {
    aeroporto_t *aeroporto;
    aviao_t *aviao;
} arg_t;

int main (int argc, char** argv) {

    // Variáveis temporais (inicio t_)
    size_t t_novo_aviao_min, t_novo_aviao_max;
    size_t t_pouso_decolagem;
    size_t t_remover_bagagens, t_inserir_bagagens;
    size_t t_bagagens_esteira, t_simulacao;

    // Variáveis discretas (inicio n_)
    size_t n_pistas, n_portoes;
    size_t n_max_avioes_esteira, n_esteiras;
    size_t n_args;

    // Variáveis de prioridade (inicio p_)
    size_t p_combustivel_min, p_combustivel_max;

    if (argc == 5) { // Argumentos sem tempos de execução
        t_novo_aviao_min = NOVO_AVIAO_MIN;
        t_novo_aviao_max = NOVO_AVIAO_MAX;
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

    } else if (argc == 14) { // Argumentos com tempos de execução
        int i = 0; // Este contador será incrementado antes de coletar um argumento
        t_novo_aviao_min = atoi(argv[++i]);
        t_novo_aviao_max = atoi(argv[++i]);
        p_combustivel_min = atoi(argv[++i]);
        p_combustivel_max = atoi(argv[++i]);
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
        printf("COMBUSTIVEL_MIN COMBUSTIVEL_MAX\n");
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
    size_t args[8] = {n_pistas, n_portoes, n_esteiras,
        n_max_avioes_esteira,
        t_pouso_decolagem, t_remover_bagagens,
        t_inserir_bagagens, t_bagagens_esteira};

        aeroporto_t* meu_aeroporto = iniciar_aeroporto(args);

        size_t contAvioes = 0;
        while (contAvioes < 10) {
            unsigned int seed = time(NULL);

            // Define uma porcentagem de combustivel de 5 a 80%
            size_t combustivel = ((size_t) COMBUSTIVEL_MIN) + rand_r(&seed) % ((size_t) COMBUSTIVEL_MAX - COMBUSTIVEL_MIN);

            // Aloca memoria dinamica na heap para struct aviao
            aviao_t *aviao = criar_aviao(contAvioes, combustivel);

            arg_t *argAux = (arg_t *) malloc(sizeof(arg_t));
            argAux->aeroporto = meu_aeroporto;
            argAux->aviao = aviao;

            // Cria a thread que representa o aviao
            pthread_create(&aviao->thread, NULL, simularAviao,(void *) argAux);

            contAvioes++;

            unsigned int seed = time(NULL);
            int wait = ((int) NOVO_AVIAO_MIN) + rand_r(&seed) % ((int) NOVO_AVIAO_MAX - NOVO_AVIAO_MIN); 
            // Esperar um tempo de NOVO_AVIAO_MIN até NOVO_AVIAO_MAX para
            // criar outro aviao (thread);
            usleep(wait * 1000);
        }

        finalizar_aeroporto(meu_aeroporto);
        return 1;
    }

    void *simularAviao(void *arg) {
        aeroporto_t meu_aeroporto = ((aeroporto_t *)arg); // mutex aqui..

    }

    void criaAviao(aviao_t *aviao) {
    }

}

