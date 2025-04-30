#include <mpi.h>
#include <stdio.h>

int main(int argc, char **argv)
{
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Verificamos que size es potencia de 2
    if ((size & (size - 1)) != 0)
    {
        if (!rank)
            fprintf(stderr, "Number of processes must be a power of 2\n");
        MPI_Finalize();
        return 1;
    }

    int value = rank;
    int sum = value;

    int step = 1;
    while (step < size)
    {
        if (rank % (2 * step) == 0)
        {
            int recv_from = rank + step;
            int recv_value;
            MPI_Recv(&recv_value, 1, MPI_INT, recv_from, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            //printf("Paso: %d Proceso: %d recibe %d de: %d Sum actual: %d \n", step, rank, recv_value, recv_from, sum);
            sum += recv_value;
        }
        else if (rank % step == 0)
        {
            int send_to = rank - step;
            //printf("Paso: %d Proceso: %d envía %d a %d \n",step, rank, sum, send_to);
            MPI_Send(&sum, 1, MPI_INT, send_to, 0, MPI_COMM_WORLD);
        }
        step *= 2;
    }

    if (rank == 0)
        printf("Total sum = %d\n", sum);

    // Broadcast the total sum to all processes
    MPI_Bcast(&sum, 1, MPI_INT, 0, MPI_COMM_WORLD);
    //MPI_Barrier(MPI_COMM_WORLD);
    printf("Process %d has total sum = %d\n", rank, sum);

    MPI_Finalize();
    return 0;
}