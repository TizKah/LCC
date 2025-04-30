/* 

Ej. 4 (Primalidad). 

Escriba utilizando OpenMP una funcion que verifique si un entero es primo
(buscando divisores entre 2 y √N ). 

Su solucion deberıa andar igual o mas rapido que una version
secuencial que “corta” apenas encuentra un divisor. 

Escriba su funcion para tomar un long, i.e. un entero
de 64 bits1, y asegurese de probarla con numeros grandes (incluyendo primos, semiprimos, y pares).

*/



#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>
// #include "timing.h"

int main(int argc, char* argv[])
{
    long num = atoi(argv[1]);

    int finish = 1;
    #pragma omp parallel shared(finish)
    {
        long raiz = sqrt(num);
        #pragma omp for
        for (int i = 2; i <= raiz; i++)
        {
            if(num % i == 0){
                finish = 0;
                raiz = i+1;
            }
        }
    }

    printf("Es primo: %d\n", finish);

    return 0;
}