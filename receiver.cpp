#include "receiver.h"

#define ANSI_COLOR_RESET   "\x1b[0m"

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
            case HOSPITAL:
                answer.id = status.MPI_SOURCE;
                answer.time = buf;
                
                if (STATE!= 7 && STATE!=8) {
                    ++CLOCK;
                    send_msg(CLOCK, answer.id, OKHOSPITAL);
                } else if (wait_for_resource == 2 && answer.time < LAST_REQ) {
                    ++CLOCK;
                    send_msg(CLOCK, answer.id, OKHOSPITAL);
                } else if (wait_for_resource == 2 && answer.time == LAST_REQ) {
                    if (rank < answer.id) {
                        ++CLOCK;
                        send_msg(CLOCK, answer.id, OKHOSPITAL);
                    } else {
                        hsp_queue.push_back(Info {answer.id, answer.time});
                    }
                } else {
                    hsp_queue.push_back(Info {answer.id, answer.time});
                }

                stateMutex.unlock();

                break;

            case OKHOSPITAL:
                hospital_approved++;

                if (hospital_approved >= size - hospital) {
                    hospital_approved = 0;
                    STATE = 7;
                    stateMutex.unlock();
                    canGoFurther.notify_all();
                }
                stateMutex.unlock();
                break;

            default:
                stateMutex.unlock();
                break;
        }

    }
}