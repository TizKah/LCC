/* 
    Implemente una librerıa de variables de condicion usando semaforos.
*/

#include <semaphore.h>
#include "semv.h"

struct condition {
    void* next;  /* doubly linked list implementation of */
    void* prev;  /* queue for blocked threads */ 
    sem_t mx; /*protects queue */
};


int sem_cond_wait(struct condition *cv, sem_t *mx){
    mutex_acquire(&c->listLock);  /* protect the queue */
    enqueue (&c->next, &c->prev, thr_self()); /* enqueue */
    mutex_release (&c->listLock); /* we're done with the list */
  
    /* The suspend and release_mutex() operation should be atomic */
    release_mutex (mx));
    thr_suspend (self);  /* Sleep 'til someone wakes us */
  
    mutex_acquire (mx); /* Woke up -- our turn, get resource lock */
  
    return;
}
int sem_cond_signal(){

}
int sem_cond_broadcast(){

}