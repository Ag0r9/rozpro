#include <mpi.h>
#include <stdio.h>
#include <string.h>

#include "main.h"
#include "queue_fun.h"

int size, rank;
int CLOCK;

int main(int argc, char** argv) {
    MPI_Status status;
    int buf;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    CLOCK = 0;

    if (rank % 2 == 0) {
        for(int i=0; i<size; ++i) {
            buf = CLOCK;
            ++CLOCK;
            MPI_Send(&buf, 1, MPI_INT, i, RDY, MPI_COMM_WORLD);
        }
    }
    else {
        MPI_Recv(&buf, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        printf("Rank %d out of %d processors, message tag: %d, message source: %d\n", rank, size, status.MPI_TAG, status.MPI_SOURCE);
        switch ( status.MPI_TAG ) {
            case RDY:

            break;
        }
    }
    

    // Finalize the MPI environment.
    MPI_Finalize();
}