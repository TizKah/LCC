#ifndef __SEMV_H__
#define __SEMV_H__
#include <semaphore.h>


typedef semv_cond_t;
typedef semv_sem_t;

/* 

    Pone al hilo que la invoca a la espera de una condición liberando el lock
    que tiene.

*/
int sem_cond_wait(semv_cond_t cond, semv_sem_t sem);

/* 

    Despierta a un hilo que esté a la espera

*/
int sem_cond_signal(semv_cond_t cond);


/* 

    Despierta a todos los hilos esperando.

*/
int sem_cond_broadcast(semv_cond_t cond);


#endif