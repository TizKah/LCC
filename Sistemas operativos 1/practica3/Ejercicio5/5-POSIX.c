#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include "rwl.h"

/*

El problema de los lectores y escritores consiste en M hilos lectores y N escritores
tratando de acceder a un arreglo en memoria compartida con las siguientes restricciones:

- No puede haber un lector accediendo al arreglo al mismo tiempo que un escritor.
- Solo puede haber un escritor a la vez.
- Varios lectores pueden acceder al arreglo simultaneamente.

*/

#define M 128
#define N 190
#define ARRLEN 10240
int arr[ARRLEN];
//rwl_t rwl = RWL_INIT; 
rwl_t *rwl; 

pthread_mutex_t reader_mutex[M];

void *escritor(void *arg)
{
    int i;
    int num = arg - (void *)0;
    while (1)
    {
        usleep(random() % 3000);
        wrlock(rwl);
        printf("Escritor %d escribiendo\n", num);
        for (i = 0; i < ARRLEN; i++)
            arr[i] = num;
        printf("Escritor %d terminó\n", num);
        wrunlock(rwl);
    }
    return NULL;
}

void *lector(void *arg)
{
    int v, i;
    int num = arg - (void *)0;
    while (1)
    {
        usleep(random() % 3000);;
        rdlock(rwl);
        printf("Lector %d, start\n", num);
        v = arr[0];
        for (i = 1; i < ARRLEN; i++)
        {
            if (arr[i] != v)
                break;
        }
        if (i < ARRLEN)
            printf("Lector %d, error de lectura\n", num);
        else
            printf("Lector %d, dato %d\n", num, v);
        printf("Lector %d, end\n", num);
        rdunlock(rwl);
    }
    return NULL;
}

int main()
{
    pthread_t lectores[M], escritores[N];
    rwl = rwl_init();
    for (int j = 0; j < M; j++)
        pthread_mutex_init(&(reader_mutex[j]), NULL);

    int i;
    for (i = 0; i < M; i++)
        pthread_create(&lectores[i], NULL, lector, i + (void *)0);
    for (i = 0; i < N; i++)
        pthread_create(&escritores[i], NULL, escritor, i + (void *)0);

    pthread_join(lectores[0], NULL); /* Espera para siempre */
    return 0;
}