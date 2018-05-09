#include "aeroporto.h"
#include "fila.h"

#define NOVO_AVIAO_MIN 30000 // 30 milisegundos
#define NOVO_AVIAO_MAX 120000 // 120  milisegundos
#define COMBUSTIVEL_MIN 5 // 5%
#define COMBUSTIVEL_MAX 65 // 65%
#define TEMPO_APROXIMACAO_AERO  600000 //  600  milisegundos
#define TEMPO_POUSO_DECOLAGEM 40000 // 40 milisegundos
#define TEMPO_REMOVER_BAGAGENS 200000 // 200 milisegundos
#define TEMPO_BAGAGENS_ESTEIRA 1000000 // 1 segundo
#define TEMPO_INSERIR_BAGAGENS 300000 // 300 milisegundos 
#define TEMPO_SIMULACAO 10000000 // 10 segundos

void criarAviao(unsigned int seed, size_t idAviao, aeroporto_t *aero);
void *simularAviao(void *arg);

typedef struct {
    aeroporto_t *aeroporto;
    aviao_t *aviao;
} arg_t;

// Variáveis temporais (inicio t_)
size_t t_novo_aviao_min, t_novo_aviao_max;
size_t t_aproximacao_aero, t_pouso_decolagem;
size_t t_remover_bagagens, t_inserir_bagagens;
size_t t_simulacao, t_bagagens_esteira;

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
        t_bagagens_esteira = TEMPO_BAGAGENS_ESTEIRA;
        t_inserir_bagagens = TEMPO_INSERIR_BAGAGENS;
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
        t_bagagens_esteira = atoi(argv[++i]);
        t_inserir_bagagens = atoi(argv[++i]);
        n_portoes = atoi(argv[++i]);
        n_max_avioes_esteira = atoi(argv[++i]);
        n_esteiras = atoi(argv[++i]);
        t_simulacao = atoi(argv[++i]);

    } else { // Número incorreto de argumentos
        printf("Todas as entradas são inteiros positivos!!\nUso:\n");
        printf("./aeroporto  NOVO_AVIAO_MIN  NOVO_AVIAO_MAX\n");
        printf("COMBUSTIVEL_MIN COMBUSTIVEL_MAX   TEMPO_APROXIMACAO_AERO\n");
        printf("TEMPO_POUSO_DECOLAGEM  NUMERO_PISTAS  TEMPO_REMOVER_BAGAGENS\n");
        printf("TEMPO_BAGAGENS_ESTEIRA TEMPO_INSERIR_BAGAGENS\nNUMERO_PORTOES  MAXIMO_AVIOES_ESTEIRA\n");
        printf("TEMPO_BAGAGENS_ESTEIRA  NUMERO_ESTEIRAS  TEMPO_SIMULACAO\n");
        printf("----------OU----------\n");
        printf("./airport  NUMERO_PISTAS  NUMERO_PORTOES  MAXIMO_AVIOES_ESTEIRA  NUMERO_ESTEIRAS\n");
        return 0;
    }

    // Impressão com os parâmetros selecionados para simulação
    printf("Simulação iniciada com tempo total: %lu milisegundos\n", t_simulacao/1000);
    printf("Tempo para criação de aviões: %lu - %lu milisegundos\n", t_novo_aviao_min/1000, t_novo_aviao_max/1000);
    printf("Número de pistas de pouso: %lu\n", n_pistas);
    printf("Tempo de aproximacao do aeroporto = %lu milisegundos\n", t_aproximacao_aero/1000);
    printf("Tempo de pouso e decolagem: %lu milisegundos\n", t_pouso_decolagem/1000);
    printf("Número de portões de embarque: %lu\n", n_portoes);
    printf("Tempo de inserção (%lu milisegundos) e remoção (%lu milisegundos) de bagagens\n", t_inserir_bagagens/1000, t_remover_bagagens/1000);
    printf("Número de esteiras: %lu, com %lu aviões por esteira\n", n_esteiras, n_max_avioes_esteira);

    // Inicialização do aeroporto
    size_t args[9] = {n_pistas, n_portoes, n_esteiras, n_max_avioes_esteira, t_aproximacao_aero, t_pouso_decolagem, t_remover_bagagens, t_bagagens_esteira, t_inserir_bagagens};

    aeroporto_t* aero = iniciar_aeroporto(args);

    size_t contAvioes = 0;
    int wait;
    int nAvioes = 10;
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
    aeroporto_t *aeroporto =  args->aeroporto;
    aviao_t *aviao = args->aviao;
    // Liberação de memoria de args
    free(args);

    // Insere um aviao em uma fila de pouso de uma pista
    size_t idPista = solicitarPista(aeroporto, aviao, 0);
    
    // Caso a verificação resulte true então ele realiza o pouso dentro desta
    // Função, ou seja, pousar_aviao é chamado aqui dentro
    usarPistaSePrimeiroDaFilaPista(aeroporto, aviao, idPista, 0);

    size_t idPortao = solicitarAcoplagem(aeroporto, aviao);

    acoplarSePrimeiroDafilaDeAcoplagem(aeroporto, aviao, idPortao);

    abastecerAviao(aviao);

    transportar_bagagens(aeroporto, aviao, idPortao);

    desaclopar_aviao(aeroporto, aviao, idPortao);

    idPista = solicitarPista(aeroporto, aviao, 1);

    usarPistaSePrimeiroDaFilaPista(aeroporto, aviao, idPista, 1);
}

