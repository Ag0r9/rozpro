#include "zmienne.h"

void send_msg(int msg, int id_, int type) {
    MPI_Send(&msg, 1, MPI_INT, id_, type, MPI_COMM_WORLD);
}