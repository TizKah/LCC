#ifndef __RWL_H_
#define __RWL_H_

#include <pthread.h>
#include <semaphore.h>

#define RWL_INIT {PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER, PTHREAD_COND_INITIALIZER, 0, 0}

typedef struct _rwl_mutex {
  pthread_mutex_t rd_mutex;
  pthread_cond_t finish_reader;
  pthread_cond_t finish_writer;
  int readers;
  int waiting_writers;
} rwl_t;

rwl_t* rwl_init();
void rdlock(rwl_t* mutex);
void rdunlock(rwl_t* mutex);
void wrlock(rwl_t* mutex);
void wrunlock(rwl_t* mutex);

#endif