#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <semaphore.h>

#include "rwl.h"
#include "sem_cv.h"

rwl_t *rwl_init()
{
  rwl_t *rwl = malloc(sizeof(struct _rwl_mutex));
  cond_var_init(&(rwl->finish_reader));
  cond_var_init(&(rwl->finish_writer));
  rwl->readers = 0;
  rwl->waiting_writers = 0;
  return rwl;
}

void rdlock(rwl_t *mutex)
{
  pthread_mutex_lock(&(mutex->finish_reader->mutex));
  if (mutex->waiting_writers > 0)
    cv_wait(&(mutex->finish_writer->global_semaphore));
  
  mutex->readers += 1;
  pthread_mutex_unlock(&(mutex->finish_reader->mutex));
}

void rdunlock(rwl_t *mutex)
{
  pthread_mutex_lock(&(mutex->finish_reader->mutex));
  mutex->readers -= 1;
  if (mutex->readers == 0)
    cv_broardcast(&(mutex->finish_reader->global_semaphore));

  pthread_mutex_unlock(&(mutex->finish_reader->mutex));
}

void wrlock(rwl_t *mutex)
{
  pthread_mutex_lock(&(mutex->finish_writer->mutex));
  mutex->waiting_writers += 1;
  while (mutex->readers > 0)
    cv_wait(&(mutex->finish_reader->global_semaphore));
  mutex->waiting_writers -= 1;
}

void wrunlock(rwl_t *mutex)
{
  cv_signal(&(mutex->finish_writer->global_semaphore));
  pthread_mutex_unlock(&(mutex->finish_writer->mutex));
}