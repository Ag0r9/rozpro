#include <mpi.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "main.h"
#include "queue_fun.h"

#define MAX(a,b) (((a)>(b))?(a):(b))

int size, rank;
int CLOCK, STATE;
int hospital, second, queue_length;
int wait_for_resource = 0; // 0 - nothing, 1 = second, 2 - hospital
int is_approved, coded_msg;

int send_msg(int msg, int id_, int type) {
    msg++;
    MPI_Send(&msg, 1, MPI_INT, id_, type, MPI_COMM_WORLD);
    return msg;
}

int encode(int id, int time) {
    return id + time * 100;
}

struct Info decode(int code) {
    struct Info info;
    info.id = code % 100;
    info.time = code / 100;
    return info;
}

int main(int argc, char** argv) {
    queue_length = 8;
    hospital = 5;
    second = 3;

    struct Info info;
    struct Queue* queue;
    struct Queue* hsp_queue;
    struct Queue* sec_queue;
    
    MPI_Status status;
    int buf;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    CLOCK = 0;
    STATE = 1;

    int hsp_aproved[size];
    int sec_approved[size];
    int fight_approved[size];

    int halo[10];

    int position = -1;
    int opponent = -1;

    queue = createQueue(queue_length);
    hsp_queue = createQueue(queue_length);
    sec_queue = createQueue(queue_length);

    memset(hsp_aproved, 0, sizeof hsp_aproved);
    memset(sec_approved, 0, sizeof sec_approved);
    memset(fight_approved, 0, sizeof fight_approved);
    memset(halo, 0, sizeof halo);

    while (1) {
        switch (STATE) {
            case 1: // Process report willingness to participate in a fight.
                info.id = rank;
                info.time = CLOCK;
                enqueue(queue, info);
                sort(queue);
                for(int i=0; i<size; ++i) {
                    if (i != rank) {
                        buf = CLOCK;
                        CLOCK = send_msg(buf, i, RDY);
                    }
                }
                break;

            case 2: // Process is looking for an opponent
                position = findPosition(queue, rank);

                if (position == -1 || position % 2 == 1)
                    break;
                
                if (position == 0) {
                    info = findFellowBehindMe(queue, position);
                    if (info.id == -1)
                        break;
                    buf = CLOCK;
                    CLOCK = send_msg(buf, info.id, FGT);
                    STATE = 0;
                    position = -1;
                    printf("DD rank: %d\n", rank);
                }

                break;

            case 3: // Process and their opponent are looking for a second.
                for(int i=0; i<size; ++i) {
                        if (i != rank) {
                            buf = CLOCK;
                            CLOCK = send_msg(buf, i, SEK);
                        }
                    }
                    STATE = 0;
                    wait_for_resource = 1;
                break;

            case 4: // Process and their opponent get a second.
                STATE = 5;
                break;
                
            case 5: // Fight takes place and the second is being terminated.
                srand(time(NULL));
                int result = rand() % 2;
                if (result == 0) {
                    STATE = 1;
                    CLOCK++;
                    buf = encode(2, CLOCK);
                    MPI_Send(&buf, 1, MPI_INT, opponent, ISHURT, MPI_COMM_WORLD);
                } else {
                    STATE = 6;
                    CLOCK++;
                    buf = encode(1, CLOCK);
                    MPI_Send(&buf, 1, MPI_INT, opponent, ISHURT, MPI_COMM_WORLD);
                }

                for(int i=0; i<size; ++i) { //Processes are leaving the second.
                    if (i != rank) {
                        buf = CLOCK;
                        CLOCK = send_msg(buf, i, NOSEK);
                    }
                }
                
                wait_for_resource = 0;

                while (!isEmpty(sec_queue)) {
                    info = dequeue(sec_queue);
                    buf = CLOCK;
                    CLOCK = send_msg(buf, info.id, OKSEK);
                }

                // forget opponent
                opponent = -1;

                break;

            case 6: // Process which lost is looking for a hospital.
                printf("BBBBB");
                for(int i=0; i<size; ++i) {
                    if (i != rank) {
                        buf = CLOCK;
                        CLOCK = send_msg(buf, i, HSP);
                    }
                }
                STATE = 0;
                wait_for_resource = 2;
                break;

            case 7: // Process is staying in a hospital.
                STATE = 8;
                break;

            case 8: // Process is leaving hospital.
                for(int i=0; i<size; ++i) {
                    if (i != rank) {
                        buf = CLOCK;
                        CLOCK = send_msg(buf, i, NOHSP);
                    }
                    
                }
                
                wait_for_resource = 0;
                STATE = 1;

                while (!isEmpty(hsp_queue)) {
                    info = dequeue(hsp_queue);
                    buf = CLOCK;
                    CLOCK = send_msg(buf, info.id, OKHSP);
                }
                break;

            case 0: // Process is waiting.
                break;

            default:
                break;
        }

        // halo[STATE]++;

        // if (rank>0) {
        //     for (int i=0; i<10; ++i) {
        //         printf("%d ", halo[i]);
        //     }
        //     printf("<- rank %d\n", rank);
        // }
        

        // if (rank == 0)
        //     printf("BeforeRCV: My id %d, my state: %d\n", rank, STATE);
        struct Info answer;
        MPI_Recv(&buf, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        CLOCK = MAX(buf, CLOCK)+1;

        // if (rank == 0)
        //     printf("AfterRCV. My id %d, my state: %d, message tag: %d, from who: %d\n", rank, STATE, status.MPI_TAG, status.MPI_SOURCE);
        
        switch ( status.MPI_TAG ) {
            case RDY:
                answer.id = status.MPI_SOURCE;
                answer.time = buf;

                if (STATE != 1 && STATE != 2) {
                    CLOCK = send_msg(buf, answer.id, OK);
                } else if (STATE == 1 && answer.time < CLOCK) {
                    CLOCK = send_msg(buf, answer.id, OK);
                } else if (STATE == 1 && answer.time == CLOCK) {
                    if (rank < answer.id) {
                        CLOCK = send_msg(buf, answer.id, OK);
                    } else {
                        enqueue(queue, answer);
                        sort(queue);
                    }
                } else if (STATE == 2 || answer.time > CLOCK) {
                    enqueue(queue, answer);
                    sort(queue);
                } else {
                    enqueue(queue, answer);
                    sort(queue);
                }

                break;

            case OK:
                answer.id = status.MPI_SOURCE;
                answer.time = buf;
                is_approved = 1;

                fight_approved[answer.id] = 1;

                for (int i = 0; i<size; ++i) {
                    if (i == rank)
                        continue;
                    if (fight_approved[i] == 0) {
                        is_approved = 0;
                    }
                }

                if (is_approved == 1) {
                    STATE = 2;
                    memset(fight_approved, 0, sizeof fight_approved);
                    printf("CC rank: %d\n", rank);
                }
                break;

            case CNF:
                // id == my id => dodoarkowo zapewniam sekundata + lososwanie wyniku
                // id == my id zapisac przeciwnika
                //kazdy usuwa

                answer.id = status.MPI_SOURCE;
                answer.time = buf;

                info = deleteById(queue, answer.id);
                printf("info %d answer %d\n", answer.id, info.id);
                answer = decode(answer.time);
                info = deleteById(queue, answer.id);
                printf("info2 %d answer2 %d\n", answer.id, info.id);

                // printf("answer.id: %d, my id: %d\n", answer.id, rank);

                if (answer.id == rank) {
                    opponent = answer.id;
                    STATE = 3;
                }

                break;

            case FGT:
                answer.id = status.MPI_SOURCE;
                answer.time = buf;

                for (int i=0; i<size; ++i) {
                    if (i != rank) {
                        // printf("df %d, rank %d\n", buf, answer.id);
                        answer.time++;
                        buf = encode(answer.id, answer.time);
                        MPI_Send(&buf, 1, MPI_INT, i, CNF, MPI_COMM_WORLD);
                    } else {
                        //usunac siebie i przeciwnika
                        //go state 0
                        info = deleteById(queue, rank);
                        printf("infoaa %d answer %d\n", rank, info.id);
                        info = deleteById(queue, answer.id);
                        printf("infoaa2 %d answer %d\n", answer.id, info.id);
                        STATE = 0;
                        position = -1;
                    }
                }

                break;

            case ISHURT:
                //1 to wygral i stan 1
                // 2 to przegral i stan 6
                answer.id = status.MPI_SOURCE;
                answer.time = buf;

                answer = decode(buf);
                if (answer.id == 1) {
                    STATE = 1;
                } else {
                    STATE = 6;
                }
                break;

            case SEK:
                answer.id = status.MPI_SOURCE;
                answer.time = buf;
                
                if (STATE != 3 && STATE != 4 && STATE != 5 && (wait_for_resource != 1)) {
                    CLOCK = send_msg(buf, answer.id, OKSEK);
                } else if ((STATE == 3 || (STATE == 0 && wait_for_resource == 1)) && answer.time < CLOCK) {
                    CLOCK = send_msg(buf, answer.id, OKSEK);
                } else if ((STATE == 3 || (STATE == 0 && wait_for_resource == 1)) && answer.time == CLOCK) {
                    if (rank < answer.id) {
                        CLOCK = send_msg(buf, answer.id, OKSEK);
                    } else {
                        enqueue(sec_queue, answer);
                    }
                } else if (STATE == 4 || STATE == 5 || answer.time > CLOCK) {
                    enqueue(sec_queue, answer);
                } else {
                    enqueue(sec_queue, answer);
                }
                break;

            case OKSEK:
                answer.id = status.MPI_SOURCE;
                answer.time = buf;
                is_approved = 1;

                sec_approved[answer.id] = 1;

                for (int i = 0; i<size; ++i) {
                    if (i == rank)
                        continue;
                    if (sec_approved[i] == 0) {
                        is_approved = 0;
                    }
                }

                if (is_approved == 1) {
                    --second;
                    STATE = 4;
                    memset(sec_approved, 0, sizeof sec_approved);;
                }

                break;

            case NOSEK:
                ++second;
                break;

            case HSP:
                answer.id = status.MPI_SOURCE;
                answer.time = buf;
                
                if (STATE != 6 && STATE != 7 && STATE != 8 && (wait_for_resource != 2)) {
                    CLOCK = send_msg(buf, answer.id, OKHSP);
                } else if ((STATE == 6 || (STATE == 0 && wait_for_resource == 2)) && answer.time < CLOCK) {
                    CLOCK = send_msg(buf, answer.id, OKHSP);
                } else if ((STATE == 6 || (STATE == 0 && wait_for_resource == 2)) && answer.time == CLOCK) {
                    if (rank < answer.id) {
                        CLOCK = send_msg(buf, answer.id, OKHSP);
                    } else {
                        enqueue(hsp_queue, answer);
                    }
                } else if (STATE == 7 || STATE == 8 || answer.time > CLOCK) {
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
                    memset(hsp_aproved, 0, sizeof hsp_aproved);;
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