#include "receiver.h"

void receiverThread() {
    MPI_Status status;
    int buf;

    Info answer, info;
    int fight_approved = 0;
    int second_approved = 0;
    int hospital_approved = 0;

    while (isActive) {
        MPI_Recv(&buf, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        //stateMutex.lock();
        CLOCK = MAX(buf, CLOCK)+1;
        // stateMutex.unlock()

        switch ( status.MPI_TAG ) {
            case READY:
                answer.id = status.MPI_SOURCE;
                answer.time = buf;

                if (STATE != 1 && STATE != 2) {
                    CLOCK = send_msg(buf, answer.id, OK);
                    queue.push_back(answer);
                    sort(queue.begin(), queue.end(), sortByTime);
                } else if (STATE == 1 && answer.time < CLOCK) {
                    CLOCK = send_msg(buf, answer.id, OK);
                    queue.push_back(answer);
                    sort(queue.begin(), queue.end(), sortByTime);
                } else if (STATE == 1 && answer.time == CLOCK) {
                    if (rank < answer.id) {
                        CLOCK = send_msg(buf, answer.id, OK);
                        queue.push_back(answer);
                        sort(queue.begin(), queue.end(), sortByTime);
                    } else {
                        queue.push_back(answer);
                        sort(queue.begin(), queue.end(), sortByTime);
                    }
                } else if (STATE == 2 || answer.time > CLOCK) {
                    queue.push_back(answer);
                    sort(queue.begin(), queue.end(), sortByTime);
                } else {
                    queue.push_back(answer);
                    sort(queue.begin(), queue.end(), sortByTime);
                }

                break;

            case OK:
                answer.id = status.MPI_SOURCE;
                answer.time = buf;

                fight_approved++;

                if (fight_approved == size -1 ) {
                    fight_approved = 0;
                    STATE = 2;
                }

                break;

            case CONFIRM:
                {
                // id == my id => dodoarkowo zapewniam sekundata + lososwanie wyniku
                // id == my id zapisac przeciwnika
                //kazdy usuwa

                answer.id = status.MPI_SOURCE;
                answer.time = buf;

                deleteById(&queue, answer.id);
                answer = decode(answer.time);
                deleteById(&queue, answer.id);

                if (answer.id == rank) {
                    opponent = answer.id;
                    STATE = 3;
                }

                }
                break;

            case FIGHT:
                answer.id = status.MPI_SOURCE;
                answer.time = buf;

                for (int i=0; i<size; ++i) {
                    if (i != rank) {
                        answer.time++;
                        buf = encode(answer.id, answer.time);
                        MPI_Send(&buf, 1, MPI_INT, i, CONFIRM, MPI_COMM_WORLD);
                    } else {
                        //usunac siebie i przeciwnika
                        //go state 0
                        deleteById(&queue, rank);
                        deleteById(&queue, answer.id);
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

            case SECOND:
                answer.id = status.MPI_SOURCE;
                answer.time = buf;
                
                if (STATE != 3 && STATE != 4 && STATE != 5 && (wait_for_resource != 1)) {
                    CLOCK = send_msg(buf, answer.id, OKSECOND);
                } else if ((STATE == 3 || (STATE == 0 && wait_for_resource == 1)) && answer.time < CLOCK) {
                    CLOCK = send_msg(buf, answer.id, OKSECOND);
                } else if ((STATE == 3 || (STATE == 0 && wait_for_resource == 1)) && answer.time == CLOCK) {
                    if (rank < answer.id) {
                        CLOCK = send_msg(buf, answer.id, OKSECOND);
                    } else {
                        sec_queue.push_back(answer);
                    }
                } else if (STATE == 4 || STATE == 5 || answer.time > CLOCK) {
                    sec_queue.push_back(answer);
                } else {
                    sec_queue.push_back(answer);
                }
                break;

            case OKSECOND:
                answer.id = status.MPI_SOURCE;
                answer.time = buf;

                second_approved++;

                if (second_approved == size - 1) {
                    --second;
                    second_approved = 0;
                    STATE = 4;
                }

                break;

            case NOSECOND:
                ++second;
                break;

            case HOSPITAL:
                answer.id = status.MPI_SOURCE;
                answer.time = buf;
                
                if (STATE != 6 && STATE != 7 && STATE != 8 && (wait_for_resource != 2)) {
                    CLOCK = send_msg(buf, answer.id, OKHOSPITAL);
                } else if ((STATE == 6 || (STATE == 0 && wait_for_resource == 2)) && answer.time < CLOCK) {
                    CLOCK = send_msg(buf, answer.id, OKHOSPITAL);
                } else if ((STATE == 6 || (STATE == 0 && wait_for_resource == 2)) && answer.time == CLOCK) {
                    if (rank < answer.id) {
                        CLOCK = send_msg(buf, answer.id, OKHOSPITAL);
                    } else {
                        hsp_queue.push_back(answer);
                    }
                } else if (STATE == 7 || STATE == 8 || answer.time > CLOCK) {
                    hsp_queue.push_back(answer);
                } else {
                    hsp_queue.push_back(answer);
                }
                
                break;

            case OKHOSPITAL:
                answer.id = status.MPI_SOURCE;
                answer.time = buf;

                hospital_approved++;

                if (hospital_approved == size - 1) {
                    --hospital;
                    hospital_approved = 0;
                    STATE = 7;
                }

                break;

            case NOHOSPITAL:
                ++hospital;
                break;

            default:
                break;
        }

    }
}