#ifndef __RWL_H_
#define __RWL_H_

#include <pthread.h>
#include <semaphore.h>
#include "sem_cv.h"

#define RWL_INIT {PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER, PTHREAD_COND_INITIALIZER, 0, 0}

typedef struct _rwl_mutex {
  cond_var_t finish_reader;
  cond_var_t finish_writer;
  int readers;
  int waiting_writers;
} rwl_t;

rwl_t* rwl_init();
void rdlock(rwl_t* mutex);
void rdunlock(rwl_t* mutex);
void wrlock(rwl_t* mutex);
void wrunlock(rwl_t* mutex);

#endif