#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include "barrier.h"

/*

Una barrera para n threads tiene una unica operacion barrier_wait() que causa
que los threads se pausen hasta que todos lleguen a la barrera. Son usadas, generalmente, para asegurar
que las iteraciones de varios bucles en paralelo proceden a un mismo paso. Implemente una librerıa de
barreras, exponiendo las funciones:

*/


void barrier_init(struct barrier *b, int n)
{
    b->mutexs = malloc(sizeof(pthread_mutex_t) * n);
    for (int i = 0; i < n; i++)
        pthread_mutex_init(&(b->mutexs[i]), NULL);
    pthread_mutex_init(&(b->mutex_waiting), NULL);
    pthread_cond_init(&(b->all_in_barrier), NULL);
    b->size = (size_t)n;
    b->waiting = b->size;
}

void barrier_wait(struct barrier *b)
{
    pthread_mutex_lock(&(b->mutex_waiting));

    if (b->waiting > 1) {
        printf("Waiting...\n   Faltan %ld hilos.\n", (b->waiting));
        b->waiting -= 1;
        pthread_cond_wait(&(b->all_in_barrier), &(b->mutex_waiting));
    }
    else {
        printf("Hilo final.\n  Levantando barrera...\n");
        b->waiting = b->size;
        pthread_cond_broadcast(&(b->all_in_barrier));
    }
    
    pthread_mutex_unlock(&(b->mutex_waiting));
}