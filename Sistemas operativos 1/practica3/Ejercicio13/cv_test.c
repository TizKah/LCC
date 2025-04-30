#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include "sem_cv.h"

cond_var_t cvar;

int main()
{
    cvar = cond_var_init();

    pthread_t t1, t2, t3;
    pthread_join

    return 0;
}