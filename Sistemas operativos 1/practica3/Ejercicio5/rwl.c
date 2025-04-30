#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <semaphore.h>

#include "rwl.h"

rwl_t *rwl_init()
{
  rwl_t *rwl = malloc(sizeof(struct _rwl_mutex));
  pthread_mutex_init(&(rwl->rd_mutex), NULL);
  pthread_cond_init(&(rwl->finish_reader), NULL);
  pthread_cond_init(&(rwl->finish_writer), NULL);
  rwl->readers = 0;
  rwl->waiting_writers = 0;
  return rwl;
}

void rdlock(rwl_t *mutex)
{
  pthread_mutex_lock(&(mutex->rd_mutex));
  if (mutex->waiting_writers > 0)
    pthread_cond_wait(&(mutex->finish_writer), &(mutex->rd_mutex));
  
  mutex->readers += 1;
  pthread_mutex_unlock(&(mutex->rd_mutex));
}

void rdunlock(rwl_t *mutex)
{
  pthread_mutex_lock(&(mutex->rd_mutex));
  mutex->readers -= 1;
  if (mutex->readers == 0)
    pthread_cond_broadcast(&(mutex->finish_reader));

  pthread_mutex_unlock(&(mutex->rd_mutex));
}

void wrlock(rwl_t *mutex)
{
  pthread_mutex_lock(&(mutex->rd_mutex));
  mutex->waiting_writers += 1;
  while (mutex->readers > 0)
    pthread_cond_wait(&(mutex->finish_reader), &(mutex->rd_mutex));
  mutex->waiting_writers -= 1;
}

void wrunlock(rwl_t *mutex)
{
  pthread_cond_signal(&(mutex->finish_writer));
  pthread_mutex_unlock(&(mutex->rd_mutex));
}