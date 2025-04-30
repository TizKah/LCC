#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define PERSONAS 10000
int flag[2] = {0, 0}, turn = 0;
int asistieron = 0;

void *molinete0()
{
    for (int i = 0; i < PERSONAS / 2; i++)
    {
        flag[0] = 1;
        turn = 2;
        asm("mfence");
        while (flag[1] && (turn == 2))
            ;
        asistieron++;
        flag[0] = 0;
    }
}

void *molinete1()
{
    for (int i = 0; i < PERSONAS / 2; i++)
    {
        flag[1] = 1;
        turn = 1;
        asm("mfence");
        while (flag[0] && (turn == 1))
            ;
        asistieron++;
        flag[1] = 0;
    }
}

int main()
{
    pthread_t th_1, th_2;

    pthread_create(&th_1, NULL, molinete0, NULL);
    pthread_create(&th_2, NULL, molinete1, NULL);

    pthread_join(th_1, NULL);
    pthread_join(th_2, NULL);

    printf("Total %d - Esperadas %d\n", asistieron, PERSONAS);

    return 0;
}