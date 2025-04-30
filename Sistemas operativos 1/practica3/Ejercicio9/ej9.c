#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "barrier.h"    

#define N 1000
#define ITERS 5
#define THREADS_NUM 4

float arr1[N], arr2[N];
struct barrier my_barrier;

void calor(float *arr, int lo, int hi, float *arr2)
{
    int i;
    for (i = lo; i < hi; i++)
    {
        float m = arr[i];
        float l = i > 0 ? arr[i - 1] : m;
        float r = i < N - 1 ? arr[i + 1] : m;
        arr2[i] = m + (l - m) / 1000.0 + (r - m) / 1000.0;
    }
}

static inline int min(int a, int b){ return a < b ? a : b; }

static inline int cut(int n, int i, int m){ return i * (n / m) + min(i, n % m);}

void *thr(void *arg)
{
    int id = *((int *)arg);
    int lo = cut(N, id, THREADS_NUM), hi = cut(N, id + 1, THREADS_NUM);
    int i;
    for (i = 0; i < ITERS; i++)
    {
        barrier_wait(&my_barrier);
        calor(arr1, lo, hi, arr2);
        barrier_wait(&my_barrier);
        calor(arr2, lo, hi, arr1);
    }
    return NULL;
}

int main()
{
    pthread_t threads[THREADS_NUM];
    int thread_ids[THREADS_NUM];

    barrier_init(&my_barrier, THREADS_NUM);

    for (int i = 0; i < THREADS_NUM; i++)
    {
        thread_ids[i] = i;
        pthread_create(&threads[i], NULL, thr, &thread_ids[i]);
    }

    for (int i = 0; i < THREADS_NUM; i++)
        pthread_join(threads[i], NULL);

    return 0;
}