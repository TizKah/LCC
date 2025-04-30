#include "sem_cv.h"
#include <semaphore.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>

#include <stdio.h>

struct __thread_t
{
  struct __thread_t *next;
  sem_t sem;
};

struct __cond_var_t
{
  sem_t global_semaphore;
  thread_t waiters;
};

static thread_t thread_t_init()
{
  thread_t thread = malloc(sizeof(struct __thread_t));
  assert(thread != NULL);
  thread->next = NULL;
  sem_init(&thread->sem, 0, 0);
  return thread;
}

static void thread_t_destroy(thread_t thread)
{
  if (thread == NULL)
    return;
  while (thread != NULL)
  {
    sem_destroy(&thread->sem);
    thread_t thread_to_free = thread;
    thread = thread->next;
    free(thread_to_free);
  }
}

cond_var_t cond_var_init()
{
  cond_var_t cond_var = malloc(sizeof(struct __cond_var_t));
  assert(cond_var != NULL);
  sem_init(&cond_var->global_semaphore, 0, 1);
  cond_var->waiters = NULL;
  return cond_var;
}

void cond_var_destroy(cond_var_t cv)
{
  if (cv == NULL)
    return;
  sem_destroy(&cv->global_semaphore);
  thread_t_destroy(cv->waiters);
  free(cv);
}

void cv_wait(cond_var_t cond_var)
{
  thread_t thread = thread_t_init(cond_var->global_semaphore);
  sem_wait(&cond_var->global_semaphore);
  thread->next = cond_var->waiters;
  cond_var->waiters = thread;
  sem_post(&cond_var->global_semaphore);

  sem_wait(&thread->sem);
}

void cv_signal(cond_var_t cond_var)
{
  sem_wait(&cond_var->global_semaphore);
  if (cond_var->waiters != NULL)
  {
    sem_post(&cond_var->waiters->sem);
    // thread_t thread_to_destroy = cond_var->waiters;
    cond_var->waiters = cond_var->waiters->next;
    // thread_t_destroy(thread_to_destroy);
  }
  sem_post(&cond_var->global_semaphore);
}

void cv_broadcast(cond_var_t cond_var)
{
  sem_wait(&cond_var->global_semaphore);
  while (cond_var->waiters != NULL)
  {
    sem_post(&cond_var->waiters->sem);
    // thread_t thread_to_destroy = cond_var->waiters;
    cond_var->waiters = cond_var->waiters->next;
    // thread_t_destroy(thread_to_destroy);
  }
  sem_post(&cond_var->global_semaphore);
}