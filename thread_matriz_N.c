#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>

#define MATRIZ_TAMANHO 1000

long matrizY[MATRIZ_TAMANHO][MATRIZ_TAMANHO];
long matrizX[MATRIZ_TAMANHO][MATRIZ_TAMANHO];

void *preencheMatrizY(void *param);
void *imprimeVetor(void *param);
void *preencheMatrizX_top(void *param);
void *preencheMatrizX_sideA(void *param);
void *preencheMatrizX_sideB(void *param);
void *preencheMatrizX_middle(void *param);

pthread_t t_matrizY[MATRIZ_TAMANHO], t_imprime, t_matrizX_top, t_matrizX_sideA,
    t_matrizX_sideB, t_matrizX_middle[MATRIZ_TAMANHO - 2];

pthread_barrier_t thread_barrier;

int main(int argc, char const *argv[])
{
    srand(time(NULL));

    pthread_barrier_init(&thread_barrier, NULL, MATRIZ_TAMANHO - 1);

    for (int i = 0; i < MATRIZ_TAMANHO; i++)
    {
        int *args = malloc(sizeof(*args));
        *args = i;
        pthread_create(&t_matrizY[i], NULL, preencheMatrizY, args);
    }

    //cria a thread que preenche o topo
    pthread_create(&t_matrizX_top, NULL, preencheMatrizX_top, NULL);

    pthread_join(t_matrizX_top, NULL);
    for (int i = 0; i < MATRIZ_TAMANHO; i++)
    {
        pthread_join(t_matrizY[i], 0);
    }

    //espera a thread que preenche o topo terminar para começar as bordas

    //duas threads, uma para cada lado
    pthread_create(&t_matrizX_sideA, NULL, preencheMatrizX_sideA, NULL);
    pthread_create(&t_matrizX_sideB, NULL, preencheMatrizX_sideB, NULL);

    //espera as threads dos lados terminarem
    pthread_join(t_matrizX_sideA, NULL);
    pthread_join(t_matrizX_sideB, NULL);

    for (int i = 0; i < MATRIZ_TAMANHO - 1; i++)
    {
        int *args = malloc(sizeof(*args));
        *args = i + 1;
        pthread_create(&t_matrizX_middle[i], NULL, preencheMatrizX_middle, args);
    }

    for (int i = 0; i < MATRIZ_TAMANHO - 1; i++)
    {
        pthread_join(t_matrizX_middle[i], NULL);
    }
    

    // pthread_create(&t_imprime, NULL, imprimeVetor, NULL);
    // pthread_join(t_imprime, NULL);

    return 0;
}

void *preencheMatrizY(void *param)
{
    int thread_number = *((int *)param);

    printf("Thread: %d\n", thread_number);

    for (int j = 0; j < MATRIZ_TAMANHO; j++)
    {
        matrizY[thread_number][j] = rand() % 10;
        //matrizY[thread_number][j] = thread_number;
    }
}

void *preencheMatrizX_top(void *param)
{
    for (int i = 0; i < MATRIZ_TAMANHO; i++)
    {
        //matrizX[0][i] = 1;
        matrizX[0][i] = (rand() % 100);
    }
}

void *preencheMatrizX_sideA(void *param)
{

    for (int i = 1; i < MATRIZ_TAMANHO; i++)
    {
        matrizX[i][0] = matrizX[i - 1][0] + matrizY[i][0];
    }
}

void *preencheMatrizX_sideB(void *param)
{

    for (int i = 1; i < MATRIZ_TAMANHO; i++)
    {
        matrizX[i][MATRIZ_TAMANHO - 1] = matrizX[i - 1][MATRIZ_TAMANHO - 1] + matrizY[i][MATRIZ_TAMANHO - 1];
    }
}

void *preencheMatrizX_middle(void *param)
{
    int thread_number = *((int *)param);
    int max_value = 0;
    for (int i = 1; i < MATRIZ_TAMANHO; i++)
    {
        if(matrizX[i - 1][thread_number - 1] > matrizX[i - 1][thread_number]){
            max_value = matrizX[i - 1][thread_number - 1];
        }
        else{
            max_value = matrizX[i - 1][thread_number];
        }
        if(matrizX[i - 1][thread_number + 1] > max_value){
            max_value = matrizX[i - 1][thread_number + 1];
        }
        
        matrizX[i][thread_number] = max_value + matrizY[i][thread_number];
        pthread_barrier_wait(&thread_barrier);
    }

    printf("Thread %d pronta!\n", (thread_number));
}

//imprime vetor (Mudar para receber a matriz como parametro)
void *imprimeVetor(void *param)
{
    printf("Matriz Y\n");
    for (int i = 0; i < MATRIZ_TAMANHO; i++)
    {
        for (int j = 0; j < MATRIZ_TAMANHO; j++)
        {
            printf("%ld ", matrizY[i][j]);
        }
        printf("\n");
    }
    printf("\n");
    printf("Matriz X\n");
    for (int i = 0; i < MATRIZ_TAMANHO; i++)
    {
        for (int j = 0; j < MATRIZ_TAMANHO; j++)
        {
            printf("%ld ", matrizX[i][j]);
        }
        printf("\n");
    }
}
