#include <pthread.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>

#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define VISITORS 40

int visitantes_vistos;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int molinete()
{
    for (int i = 0; i < 20; i++)
    {
        pthread_mutex_lock(&mutex);
        visitantes_vistos++;
        pthread_mutex_unlock(&mutex);
    }
    return 0;
}

int main()
{
    pthread_t th1, th2;
    pthread_create(&th1, NULL, molinete, NULL);
    pthread_create(&th2, NULL, molinete, NULL);

    pthread_join(th1, NULL);
    pthread_join(th2, NULL);

    printf("Visitantes: %d\n", visitantes_vistos);

    return 0;
}