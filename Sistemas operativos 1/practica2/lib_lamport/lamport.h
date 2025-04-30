#ifndef __LAMPORT_H_
#define __LAMPORT_H_

typedef struct
{
    volatile int *nums;
    volatile int *eligiendo;
    volatile int size;
} _lamp_arrs;
typedef _lamp_arrs* lamp_arrs;

lamp_arrs lamport_init(int size);
int array_max(int *array, int len);
void lock(int num, lamp_arrs lamp);
void unlock(int num, lamp_arrs lamp);
void lamport_free(lamp_arrs lamport);

#endif