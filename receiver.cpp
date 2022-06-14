#include "receiver.h"

void receiverThread() {
    MPI_Status status;
    int buf;

    Info answer;
    int fight_approved = 0;
    int hospital_approved = 0;

    while (isActive) {
        MPI_Recv(&buf, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        stateMutex.lock();
        CLOCK = MAX(buf, CLOCK)+1;

        switch ( status.MPI_TAG ) {
            case READY:
                answer.id = status.MPI_SOURCE;
                answer.time = buf;

                if (STATE == 5 || (wait_for_resource == 1 && answer.time > LAST_REQ)) {
                    queue.push_back(answer);
                    printf("Time: %d, Ja %d dodaje do kolejki %d; moj time: %d, jego time %d\n", CLOCK, rank, answer.id, LAST_REQ, answer.time);
                } else if ((wait_for_resource == 0) || (wait_for_resource == 1 && answer.time <= LAST_REQ)) {
                    printf("Time: %d, Ja %d wysyłam OK do %d\n", CLOCK, rank, answer.id);
                    CLOCK++;
                    send_msg(CLOCK, answer.id, OK);
                } 
                stateMutex.unlock();
                break;

            case OK:
                fight_approved++;

                printf("Rank %d, appri: %d, size: %d, proc %d\n", rank, fight_approved, size, process);

                if (fight_approved >= size - process) {
                    fight_approved = 0;
                    STATE = 5;
                }
                stateMutex.unlock();

                break;

            case HOSPITAL:
            //     answer.id = status.MPI_SOURCE;
            //     answer.time = buf;
                
            //     if (STATE != 6 && STATE != 7 && STATE != 8 && (wait_for_resource != 2)) {
            //         CLOCK++;
            //         send_msg(CLOCK, answer.id, OKHOSPITAL);
            //     } else if ((STATE == 6 || (STATE == 0 && wait_for_resource == 2)) && answer.time < CLOCK) {
            //         CLOCK++;
            //         send_msg(CLOCK, answer.id, OKHOSPITAL);
            //     } else if ((STATE == 6 || (STATE == 0 && wait_for_resource == 2)) && answer.time == CLOCK) {
            //         if (rank < answer.id) {
            //             CLOCK++;
            //             send_msg(CLOCK, answer.id, OKHOSPITAL);
            //         } else {
            //             hsp_queue.push_back(answer);
            //         }
            //     } else if (STATE == 7 || STATE == 8 || answer.time > CLOCK) {
            //         hsp_queue.push_back(answer);
            //     } else {
            //         hsp_queue.push_back(answer);
            //     }
                stateMutex.unlock();
                break;

            case OKHOSPITAL:
                // hospital_approved++;

                // if (hospital_approved >= size - hospital) {
                //     hospital_approved = 0;
                //     STATE = 7;
                // }
                stateMutex.unlock();
                break;
                


            default:
                stateMutex.unlock();
                break;
        }

    }
}

// answer.id = status.MPI_SOURCE;
//                 answer.time = buf;

//                 printf("moje id: %d, id nadawcy: %d, jego czas %d\n", rank, answer.id, answer.time);
//                 if (STATE == 5 || (STATE ==1 && wait_for_resource != 1)) {
//                     CLOCK = send_msg(buf, answer.id, OK);
//                 } else if ((STATE == 1 || STATE == 5 || (wait_for_resource == 1)) && answer.time < LAST_REQ) {
//                     CLOCK = send_msg(buf, answer.id, OK);
//                 } else if ((STATE == 1 || STATE == 5 || (wait_for_resource == 1)) && answer.time == LAST_REQ) {
//                     if (rank < answer.id) {
//                         CLOCK = send_msg(buf, answer.id, OK);
//                     } else {
//                         queue.push_back(answer);
//                     }
//                 } else {
//                     queue.push_back(answer);
//                 }
//                 stateMutex.unlock();