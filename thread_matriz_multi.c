/*
Rafael Junio Xavier - 2018.1907.050-6
Exercício feito usando semaforos;
A fim de aprender e experimentar com várias threads, o exercício foi implementado utilizando
quase que NxN threads.
Cada thread do miolo é responsável por chamar a thread inferior esquerda para o calculo (salvo
alguns casos onde a thread está nos cantos do "miolo" do x)
Os semaforos são usados para verificar se os dados que uma thread precisa estão prontos;
Quando uma thread termina a execução ela adiciona 3 créditos em seu semaforo, com isso cada
thread que precisa dela coleta um crédito; caso a thread não tenha terminado, as que são dependentes
esperam até que ela esteja pronta.

Espero que tenha conseguido ser claro porque o código ta meio bagunçado mas funciona kkkk
*/

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include <semaphore.h>

#define MATRIZ_TAMANHO 100

long matrizY[MATRIZ_TAMANHO][MATRIZ_TAMANHO];
long matrizX[MATRIZ_TAMANHO][MATRIZ_TAMANHO];
int thread_number = 0;

void *preencheMatrizY(void *param);
void *imprimeVetor(void *param);
void *preencheMatrizX_top(void *param);
void *preencheMatrizX_sideA(void *param);
void *preencheMatrizX_sideB(void *param);
void *preencheMatrizX_middle(void *param);

// semaforo para cada thread e suas duas dependencias e mutex
sem_t avaiable[MATRIZ_TAMANHO - 1][MATRIZ_TAMANHO - 2], mutex;

pthread_t t_matrizY[MATRIZ_TAMANHO], t_imprime, t_matrizX_top, t_matrizX_sideA,
    t_matrizX_sideB, t_matrizX_middle[MATRIZ_TAMANHO - 1][MATRIZ_TAMANHO - 2];

// define a struct da posição real das threads e a posição relativa na matriz
typedef struct
{
    int pos1_thread, pos2_thread, pos1_matriz, pos2_matriz, max_value;
} pos_matrix;

//inicio da main
int main(int argc, char const *argv[])
{

        srand(time(NULL));
    // iniciando mutex
    sem_init(&mutex, 0, 1);

    // iniciando semaforo avaiable com 0 créditos
    for (int i = 0; i < MATRIZ_TAMANHO - 1; i++)
    {
        for (int j = 0; j < MATRIZ_TAMANHO - 2; j++)
        {
            sem_init(&avaiable[i][j], 0, 0);
        }
    }

    // criando threads para preencher matriz Y
    for (int i = 0; i < MATRIZ_TAMANHO; i++)
    {
        int *args = malloc(sizeof(*args));
        *args = i;
        pthread_create(&t_matrizY[i], NULL, preencheMatrizY, args);
    }

    //cria a thread que preenche o topo
    pthread_create(&t_matrizX_top, NULL, preencheMatrizX_top, NULL);

    //aguarda as threads do Y terminarem
    for (int i = 0; i < MATRIZ_TAMANHO; i++)
    {
        pthread_join(t_matrizY[i], 0);
    }

    //espera a thread que preenche o topo terminar para começar as bordas
    pthread_join(t_matrizX_top, NULL);

    //duas threads, uma para cada lado
    pthread_create(&t_matrizX_sideA, NULL, preencheMatrizX_sideA, NULL);
    pthread_create(&t_matrizX_sideB, NULL, preencheMatrizX_sideB, NULL);

    //espera as threads dos lados terminarem
    pthread_join(t_matrizX_sideA, NULL);
    pthread_join(t_matrizX_sideB, NULL);

    // criando a thread do miolo
    for (int i = 0; i < MATRIZ_TAMANHO - 2; i++)
    {
        //alocando espaço sequencia na memoria
        // sem_wait(&mutex);
        pos_matrix *pos;
        pos = malloc(sizeof(pos_matrix));

        //valores relativos da thread em relação a matriz
        pos->pos1_matriz = 1;
        pos->pos2_matriz = i + 1;
        // sem_post(&mutex);

        //Da dois créditos para as threads iniciais do miolo para que as threads subsquentes possam continuar as operações
        sem_post(&avaiable[0][i]);
        sem_post(&avaiable[0][i]);

        // Cria as threads do cabeçalho do miolo até MATRIZ_TAMANHO - 2
        pthread_create(&t_matrizX_middle[0][i], NULL, preencheMatrizX_middle, pos);
    }

    //Aguarda que todas as threads criadas estejam prontas antes de imprimir
    for (int i = 0; i < MATRIZ_TAMANHO - 1; i++)
    {
        for (int j = 0; j < MATRIZ_TAMANHO - 2; j++)
        {
            pthread_join(t_matrizX_middle[i][j], NULL);
            printf("Joining thread.... \n");
        }
    }

    // Cria uma thread que vai imprimir os vetores
    printf("Tudo pronto, imprima.... \n");
    pthread_create(&t_imprime, NULL, imprimeVetor, NULL);
    pthread_join(t_imprime, NULL);
    return 0;
}

//preenche a MatrizY com o
void *preencheMatrizY(void *param)
{
    int thread_number = *((int *)param);

    printf("Thread: %d\n", thread_number);

    for (int j = 0; j < MATRIZ_TAMANHO; j++)
    {
        matrizY[thread_number][j] = rand() % 10;
    }
}

//preenche o topo da MatrizX
void *preencheMatrizX_top(void *param)
{
    for (int i = 0; i < MATRIZ_TAMANHO; i++)
    {
        matrizX[0][i] = rand() % 100;
    }
}

//preenche o lado da matrizX
void *preencheMatrizX_sideA(void *param)
{

    for (int i = 1; i < MATRIZ_TAMANHO; i++)
    {
        matrizX[i][0] = matrizX[i - 1][0] + matrizY[i][0];
    }
}

//preenche o lado da matrizX
void *preencheMatrizX_sideB(void *param)
{

    for (int i = 1; i < MATRIZ_TAMANHO; i++)
    {
        matrizX[i][MATRIZ_TAMANHO - 1] = matrizX[i - 1][MATRIZ_TAMANHO - 1] + matrizY[i][MATRIZ_TAMANHO - 1];
    }
}

//função que preenche o miolo
void *preencheMatrizX_middle(void *param)
{

    pos_matrix *pos_a = malloc(sizeof(pos_a));
    pos_a = (pos_matrix *)param;
    pos_a->pos1_thread = pos_a->pos1_matriz - 1;
    pos_a->pos2_thread = pos_a->pos2_matriz - 1;
    pos_a->max_value = 0;

    if ((pos_a->pos2_thread == 0) && (pos_a->pos1_thread > 0))
    {
        // printf("THREAD: %d -- %d Esperando thread meio: [%d][%d]!\n", pos_a->pos1_thread, pos_a->pos2_thread, pos_a->pos1_thread - 1, pos_a->pos2_thread);
        sem_wait(&avaiable[pos_a->pos1_thread - 1][pos_a->pos2_thread]);
    }
    else if ((pos_a->pos2_thread == MATRIZ_TAMANHO - 2))
    {

        // printf("THREAD: %d -- %d Esperando thread esquerda: [%d][%d]!\n", pos_a->pos1_thread, pos_a->pos2_thread, pos_a->pos1_thread - 1, pos_a->pos2_thread - 1);
        sem_wait(&avaiable[pos_a->pos1_thread - 1][pos_a->pos2_thread - 1]);

        // printf("THREAD: %d -- %d Esperando thread meio: [%d][%d]!\n", pos_a->pos1_thread, pos_a->pos2_thread, pos_a->pos1_thread - 1, pos_a->pos2_thread);
        sem_wait(&avaiable[pos_a->pos1_thread - 1][pos_a->pos2_thread]);
    }
    else if ((pos_a->pos2_thread > 0) && (pos_a->pos1_thread > 0))
    {
        // printf("THREAD: %d -- %d Esperando thread esquerda: [%d][%d]!\n", pos_a->pos1_thread, pos_a->pos2_thread, pos_a->pos1_thread - 1, pos_a->pos2_thread - 1);
        sem_wait(&avaiable[pos_a->pos1_thread - 1][pos_a->pos2_thread - 1]);

        // printf("THREAD: %d -- %d Esperando thread meio: [%d][%d]!\n", pos_a->pos1_thread, pos_a->pos2_thread, pos_a->pos1_thread - 1, pos_a->pos2_thread);
        sem_wait(&avaiable[pos_a->pos1_thread - 1][pos_a->pos2_thread]);
    }

    printf("get max\n");

    if (matrizX[pos_a->pos1_matriz - 1][pos_a->pos2_matriz - 1] > matrizX[pos_a->pos1_matriz - 1][pos_a->pos2_matriz])
    {
        pos_a->max_value = matrizX[pos_a->pos1_matriz - 1][pos_a->pos2_matriz - 1];
    }
    else
    {
        pos_a->max_value = matrizX[pos_a->pos1_matriz - 1][pos_a->pos2_matriz];
    }
    if (matrizX[pos_a->pos1_matriz - 1][pos_a->pos2_matriz + 1] > pos_a->max_value)
    {
        pos_a->max_value = matrizX[pos_a->pos1_matriz - 1][pos_a->pos2_matriz + 1];
    }

    matrizX[pos_a->pos1_matriz][pos_a->pos2_matriz] = pos_a->max_value + matrizY[pos_a->pos1_matriz][pos_a->pos2_matriz];

    printf("THREAD: %d -- %d -- Escreveu na matriz\n", pos_a->pos1_thread, pos_a->pos2_thread);
    //liberando créditos para as threads dependentes
    sem_post(&avaiable[pos_a->pos1_thread][pos_a->pos2_thread]);
    sem_post(&avaiable[pos_a->pos1_thread][pos_a->pos2_thread]);

    if (((pos_a->pos2_thread) == 0))
    {
        //threads da primeira coluna não chamam outras threads
        free(pos_a);
        pthread_exit(0);
    }
    else if ((pos_a->pos1_thread) == MATRIZ_TAMANHO - 2)
    {
        //threads da ultima linha não chamam mais threads
        free(pos_a);
        pthread_exit(0);
    }
    else
    {
        //aloca na memoria posicoes para a thread a ser criada
        pos_matrix *pos_b;
        pos_b = malloc(sizeof(pos_b));

        //guarda o valor relativo a matriz da nova thread
        pos_b->pos1_matriz = pos_a->pos1_matriz + 1;
        pos_b->pos2_matriz = pos_a->pos2_matriz - 1;

        //cria a nova thread
        pthread_create(&t_matrizX_middle[pos_a->pos1_thread + 1][pos_a->pos2_thread - 1], NULL, preencheMatrizX_middle, pos_b);

        if (pos_a->pos2_thread == (MATRIZ_TAMANHO - 3))
        {
            pos_matrix *pos_c;
            pos_c = malloc(sizeof(pos_c));
            pos_c->pos1_matriz = pos_a->pos1_matriz + 1;
            pos_c->pos2_matriz = pos_a->pos2_matriz;
            // printf("THREAD: %d -- %d CRIOU CANTO\n", pos_a->pos1_thread + 1, pos_a->pos2_thread);
            pthread_create(&t_matrizX_middle[pos_a->pos1_thread + 1][pos_a->pos2_thread], NULL, preencheMatrizX_middle, pos_c);
        }
    }
    //libera memoria da pos_a
    free(pos_a);
    pthread_exit(0);
}

//imprime vetor (Mudar para receber a matriz como parametro)
void *imprimeVetor(void *param)
{
    for (int i = 0; i < MATRIZ_TAMANHO; i++)
    {
        for (int j = 0; j < MATRIZ_TAMANHO; j++)
        {
            printf("%ld ", matrizY[i][j]);
        }
        printf("\n");
    }
    printf("\n");
    for (int i = 0; i < MATRIZ_TAMANHO; i++)
    {
        for (int j = 0; j < MATRIZ_TAMANHO; j++)
        {
            printf("%ld ", matrizX[i][j]);
        }
        printf("\n");
    }
}
