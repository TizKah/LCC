#include <mpi.h>
#include <stdio.h>

int main(int argc, char **argv)
{
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int value = rank;
    int sum = value;
    int recv_value;
    // Calculamos hacia qué proceso enviar el valor y de cuál recibirlo
    int send_to = (rank + 1) % size;
    int recv_from = (rank - 1 + size) % size;
    
    // Es como una conexión ejemplificada por un grafo conexo completo
    for (int i = 0; i < size - 1; i++)
    {

        // Enviamos y recibimos los valores
        MPI_Send(&value, 1, MPI_INT, send_to, 0, MPI_COMM_WORLD);
        MPI_Recv(&recv_value, 1, MPI_INT, recv_from, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        sum += recv_value;
        //printf("Proceso: %d Recibió: %d Del proceso: %d Suma: %d\n", rank, recv_value, recv_from, sum);
        //printf("Proceso: %d Mandó: %d Al proceso: %d Suma: %d\n", rank, value, send_to, sum);
        value = recv_value;
    }
    MPI_Barrier(MPI_COMM_WORLD);
    printf("Process %d has total sum = %d\n", rank, sum);

    MPI_Finalize();
    return 0;
}