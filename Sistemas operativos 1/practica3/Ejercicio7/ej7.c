#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

pthread_mutex_t protect_resources = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t can_continue = PTHREAD_COND_INITIALIZER;

sem_t tabaco, papel, fosforos, otra_vez;
void agente()
{
    while (1)
    {
        sem_wait(&otra_vez);
        int caso = random() % 3;
        if (caso != 0)
            sem_post(&fosforos);
        if (caso != 1)
            sem_post(&papel);
        if (caso != 2)
            sem_post(&tabaco);
    }
}
void fumar(int fumador)
{
    printf("Fumador %d: Puf! Puf! Puf!\n", fumador);
    usleep(1);
}
void *fumador1(void *arg)
{
    while (1)
    {
        sem_wait(&papel);
        if (sem_trywait(&tabaco) != 0)
        {
            sem_post(&papel);
        }
        else
        {
            printf("Fumador 1 recibió papel.\n");
            printf("Fumador 1 recibió tabaco.\n");
            fumar(1);
            sem_post(&otra_vez);
        }
    }
}
void *fumador2(void *arg)
{
    while (1)
    {
        sem_wait(&tabaco);
        if (sem_trywait(&fosforos) != 0)
        {
            sem_post(&tabaco);
        }
        else
        {
            printf("Fumador 2 recibió tabaco.\n");
            printf("Fumador 2 recibió fosforos.\n");
            fumar(2);
            sem_post(&otra_vez);
        }
    }
}
void *fumador3(void *arg)
{
    while (1)
    {   
        sem_wait(&papel);
        if (sem_trywait(&fosforos) != 0)
        {
            sem_post(&papel);
        }
        else
        {
            printf("Fumador 3 recibió papel.\n");
            printf("Fumador 3 recibió fosforos.\n");
            fumar(3);
            sem_post(&otra_vez);
        }
    }
}
int main()
{
    pthread_t s1, s2, s3;
    sem_init(&tabaco, 0, 0);
    sem_init(&papel, 0, 0);
    sem_init(&fosforos, 0, 0);
    sem_init(&otra_vez, 0, 1);
    pthread_create(&s1, NULL, fumador1, NULL);
    pthread_create(&s2, NULL, fumador2, NULL);
    pthread_create(&s3, NULL, fumador3, NULL);
    agente();
    return 0;
}
