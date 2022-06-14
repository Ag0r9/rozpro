#include "receiver.h"

void receiverThread() {
    MPI_Status status;
    int buf;

    Info answer;
    int hospital_approved = 0;

    while (isActive) {
        MPI_Recv(&buf, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        stateMutex.lock();
        CLOCK = MAX(buf, CLOCK)+1;

        switch ( status.MPI_TAG ) {
            // case READY:
            //     // answer.id = status.MPI_SOURCE;
            //     // answer.time = buf;

            //     // buf = CLOCK;

                // if (STATE != 1 && (wait_for_resource != 1)) {
                //     CLOCK = send_msg(buf, answer.id, OK);
                // } else if ((STATE == 1 || (wait_for_resource == 1)) && answer.time < CLOCK) {
                //     CLOCK = send_msg(buf, answer.id, OK);
                // } else if ((STATE == 1 || (wait_for_resource == 1)) && answer.time == CLOCK) {
                //     if (rank < answer.id) {
                //         CLOCK = send_msg(buf, answer.id, OK);
                //     } else {
                //         queue.push_back(answer);
                //     }
                // } else {
                //     queue.push_back(answer);
                // }

            //     break;

            // case OK:
            //     // fight_approved++;

            //     // if (fight_approved >= size - process) {
            //     //     fight_approved = 0;
            //     //     STATE = 5;
            //     // }

            //     break;

            case HOSPITAL:
                printf("prosi o hospital %d\n", status.MPI_SOURCE);
                answer.id = status.MPI_SOURCE;
                answer.time = buf;
                
                if (STATE!=7 && STATE!=8) {
                    ++CLOCK;
                    printf("GICIOR\n");
                    send_msg(CLOCK, answer.id, OKHOSPITAL);
                } else if ((STATE == 6 || (wait_for_resource == 2)) && answer.time < LAST_REQ) {
                    ++CLOCK;
                    printf("GICIOR\n");
                    send_msg(CLOCK, answer.id, OKHOSPITAL);
                } else if ((STATE == 6 || (wait_for_resource == 2)) && answer.time == LAST_REQ) {
                    if (rank < answer.id) {
                        ++CLOCK;
                        send_msg(CLOCK, answer.id, OKHOSPITAL);
                    } else {
                        hsp_queue.push_back(Info {answer.id, answer.time} );
                    }
                } else {
                    printf("NIEGICIOR\n");
                    hsp_queue.push_back(Info {answer.id, answer.time});
                }

                stateMutex.unlock();

                break;

            case OKHOSPITAL:
                hospital_approved++;
                // printf("Rank %d, il approve %d\n", rank, hospital_approved);

                if (hospital_approved >= size - hospital) {
                    hospital_approved = 0;
                    STATE = 7;
                    stateMutex.unlock();
                    canGoFurther.notify_all();
                }
                stateMutex.unlock();
                break;


            default:
                break;
        }

    }
}