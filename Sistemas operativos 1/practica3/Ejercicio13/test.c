#include "sem_cv.h"
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

#define NUM_THREADS 10

cond_var_t cond_var;

void *thread_function(void *arg)
{
  int thread_id = arg - (void*)0;
  printf("Thread %d started\n", thread_id);

  cv_wait(cond_var);

  printf("Thread %d finished\n", thread_id);
  return NULL;
}

int main()
{
  cond_var = cond_var_init();

  pthread_t threads[NUM_THREADS];
  for (int i = 0; i < NUM_THREADS; i++)
    pthread_create(&threads[i], NULL, thread_function, i + (void*)0);

  sleep(3); // Simulate work being done
  printf("Broadcasting\n");
  cv_signal(cond_var);
  cv_broadcast(cond_var);

  for (int i = 0; i < NUM_THREADS; i++)
    pthread_join(threads[i], NULL);

  cond_var_destroy(cond_var);
  return 0;
}
