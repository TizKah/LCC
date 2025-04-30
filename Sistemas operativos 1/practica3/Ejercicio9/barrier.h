#ifndef __BARRIER_H__
#define __BARRIER_H__

#include <pthread.h>
struct barrier
{
    pthread_mutex_t *mutexs;
    pthread_mutex_t mutex_waiting;
    pthread_cond_t all_in_barrier;
    size_t size;
    size_t waiting;
};

void barrier_init(struct barrier *b, int n);
void barrier_wait(struct barrier *b);


#endif