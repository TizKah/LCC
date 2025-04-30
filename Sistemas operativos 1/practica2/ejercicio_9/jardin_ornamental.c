#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define PERSONAS 10000000
int asistieron = 0;

void *molinete()
{
    for (int i = 0; i < PERSONAS / 2; i++)
        asistieron = asistieron + 1;
}

int main()
{
    pthread_t th_1, th_2;

    pthread_create(&th_1, NULL, molinete, NULL);
    pthread_create(&th_2, NULL, molinete, NULL);

    pthread_join(th_1, NULL);
    pthread_join(th_2, NULL);

    printf("Total %d - Esperadas %d\n", asistieron, PERSONAS);

    return 0;
}