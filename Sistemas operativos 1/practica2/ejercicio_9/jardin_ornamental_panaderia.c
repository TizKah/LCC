#include <unistd.h>
#include <pthread.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define false 0
#define true 1
typedef int bool;

#define VISITORS 40
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
  eligiendo[num] = true;
  nums[num] = 1 + array_max(nums, N_THREADS);
  eligiendo[num] = false;

  for (int i = 0; i < N_THREADS; i++)
  {
    while (eligiendo[i])
      ;
    while ((nums[i] != 0) && ((nums[i] < nums[num]) || (nums[i] == nums[num])) && i < num)
      ;
  }
}

void unlock(int num)
{
  nums[num] = 0;
}

void molinete(int thread_num)
{
  printf("thread num: %d\n", thread_num);
  for (int i = 0; i < 20; i++)
  {
    printf("%d: %d\n", thread_num, i);
    lock(thread_num);
    visitantes_vistos++;
    unlock(thread_num);
  }
  printf("%d end\n", thread_num);
}

int main()
{
  pthread_t *threads[N_THREADS];

  for (int i = 0; i < N_THREADS; i++)
    pthread_create(&threads[i], NULL, molinete, i);

  for (int i = 0; i < N_THREADS; i++)
    pthread_join(&threads[i], NULL);

  printf("Visitantes: %d\n", visitantes_vistos);

  return 0;
}