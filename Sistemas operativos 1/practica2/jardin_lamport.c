#include <unistd.h>
#include <pthread.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define false 0
#define true 1
typedef int bool;

#define VISITORS 70000
#define N_THREADS 2

int nums[N_THREADS], visitantes_vistos = 0;
bool eligiendo[N_THREADS];

int array_max(int *array, int len)
{
  int max = 0;
  for (int i = 0; i < len; i++)
    if (max < array[i])
      max = array[i];
  return max;
}

void lock(int num)
{
  asm("mfence");
  eligiendo[num] = true;
  asm("mfence");
  nums[num] = 1 + array_max(nums, N_THREADS);
  asm("mfence");
  eligiendo[num] = false;

  for (int i = 0; i < N_THREADS; i++)
  {
    while (eligiendo[i])
      ;
    while ((nums[i] != 0) && ((nums[i] < nums[num]) || ((nums[i] == nums[num]) && (i < num))))
      ;
  }
}

void unlock(int num)
{
  asm("mfence");
  nums[num] = 0;
}

void *molinete(void *thread_num)
{
  int t_num = thread_num - (void *)0;
  for (int i = 0; i < VISITORS / N_THREADS; i++)
  {
    lock(t_num);
    visitantes_vistos++;
    unlock(t_num);
  }
  return NULL;
}

int main()
{
  pthread_t *threads[N_THREADS];

  for (int i = 0; i < N_THREADS; i++)
    pthread_create(&threads[i], NULL, molinete, (void *)i);

  for (int i = 0; i < N_THREADS; i++)
    pthread_join(threads[i], NULL);

  printf("Visitantes: %d\n", visitantes_vistos);

  return 0;
}