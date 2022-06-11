#include <mpi.h>
#include <stdio.h>
#include <string.h>

#include "main.h"
#include "queue_fun.h"

#define MAX(a,b) (((a)>(b))?(a):(b))

// MPI RECV TIME TP BUF??
// KIEDY WYCHODZI Z KOELJKI

int size, rank;
int CLOCK, STATE;
int hospital, second, queue_length;
int wait_for_resource = 0; // 0 - nothing, 1 = second, 2 - hospital
int is_approved;

int send_msg(int msg, int id_, int type) {
    msg++;
    MPI_Send(&msg, 1, MPI_INT, id_, type, MPI_COMM_WORLD);
    return msg;
}

int main(int argc, char** argv) {
    queue_length = 8;
    hospital = 5;
    second = 3;

    struct Info info;
    struct Queue* queue;
    struct Queue* hsp_queue;
    struct Queue* sec_queue;
    queue = createQueue(queue_length);
    hsp_queue = createQueue(queue_length);
    sec_queue = createQueue(queue_length);
    
    MPI_Status status;
    int buf;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    CLOCK = 0;
    STATE = 1;

    int hsp_aproved[size];
    int sec_approved[size];

    memset(hsp_aproved, 0, sizeof hsp_aproved);
    memset(sec_approved, 0, sizeof sec_approved);

    while (1) {
        switch (STATE) {
            case 1: // Process report willingness to participate in a fight.
                for(int i=0; i<size; ++i) {
                    if (i != rank) {
                        buf = CLOCK;
                        CLOCK = send_msg(buf, i, RDY);
                    } else {
                        info.id = rank;
                        info.time = CLOCK;
                        enqueue(queue, info);
                        sort(queue);
                    }
                }
                STATE = 2;
                break;

            case 2: // Process is looking for an opponent
                info = front(queue);
                if (info.id == rank) {
                    for(int i=0; i<size; ++i) {
                        if (i != rank) {
                            buf = CLOCK;
                            CLOCK = send_msg(buf, i, CNF);
                        } else {
                            info = dequeue(queue);
                        }
                    }
                    STATE = 5;
                }
                break;

            case 3: // Process and their opponent are looking for a second.
                break;

            case 4: // Process and their opponent get a second.
                break;

            case 5: // Fight takes place and the second is being terminated.
                srand(time(0));
                int result = rand() % 2;
                if (result == 0) {
                    STATE = 1;
                } else {
                    STATE = 6;
                }
                break;

            case 6: // Process which lost is looking for a hospital.
                for(int i=0; i<size; ++i) {
                    if (i != rank) {
                        buf = CLOCK;
                        CLOCK = send_msg(buf, i, HSP);
                    }
                }
                info.id = rank;
                info.time = CLOCK;
                enqueue(hsp_queue, info);
                STATE = 0;
                wait_for_resource = 2;
                break;

            case 7: // Process is staying in a hospital.
                wait(1);
                STATE = 8;
                break;

            case 8: // Process is leaving hospital.
                for(int i=0; i<size; ++i) {
                    if (i != rank) {
                        buf = CLOCK;
                        CLOCK = send_msg(buf, i, NOHSP);
                    }
                }
                STATE = 1;
                break;

            case 0: // Process is waiting.
                break;

            default:
                break;
        }

        struct Info answer;
        MPI_Recv(&buf, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        CLOCK = MAX(buf, CLOCK)+1;
        printf("Rank %d out of %d processors, message tag: %d, message source: %d\n", rank, size, status.MPI_TAG, status.MPI_SOURCE);
        switch ( status.MPI_TAG ) {
            case RDY:
                answer.id = status.MPI_SOURCE;
                answer.time = buf;
                enqueue(queue, answer);
                sort(queue);
                break;

            case CNF:
                answer = dequeue(queue);
                break;

            case FGT:
                break;

            case SEK:
                break;

            case OKSEK:
                break;

            case NOSEK:
                break;

            case HSP:
                answer.id = status.MPI_SOURCE;
                answer.time = buf;
                
                // MOZE BYC BLAD
                if (STATE != 5 && STATE != 6 && STATE != 7 && (STATE != 0 && wait_for_resource != 2)) {
                    CLOCK = send_msg(buf, answer.id, OKHSP);
                } else if ((STATE == 5 || (STATE == 0 && wait_for_resource == 2)) && answer.time < CLOCK) {
                    CLOCK = send_msg(buf, answer.id, OKHSP);
                } else if ((STATE == 5 || (STATE == 0 && wait_for_resource == 2))&& answer.time == CLOCK) {
                    if (rank < answer.id) {
                        CLOCK = send_msg(buf, answer.id, OKHSP);
                    } else {
                        enqueue(hsp_queue, answer);
                    }
                } else if (STATE == 6 || STATE == 7 || answer.time > CLOCK) {
                    enqueue(hsp_queue, answer);
                } else {
                    enqueue(hsp_queue, answer);
                }
                
                break;

            case OKHSP:
                answer.id = status.MPI_SOURCE;
                answer.time = buf;
                is_approved = 1;

                hsp_aproved[answer.id] = 1;

                for (int i = 0; i<size; ++i) {
                    if (i == rank)
                        continue;
                    if (hsp_aproved[i] == 0) {
                        is_approved = 0;
                    }
                }

                if (is_approved == 1) {
                    --hospital;
                    STATE = 7;
                }

                break;

            case NOHSP:
                ++hospital;
                break;

            default:
                break;
        }
    
    }
    
    MPI_Finalize();
}