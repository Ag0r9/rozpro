#include "main.h"

std::mutex stateMutex;
std::condition_variable canGoFurther;

int size, rank;
int CLOCK, STATE;
int hospital, second;
int wait_for_resource = 0; // 0 - nothing, 1 = second, 2 - hospital
int is_approved, coded_msg;
int position = -1;
int opponent = -1;
bool isActive = true;

std::vector<Info> queue;
std::vector<Info> hsp_queue;
std::vector<Info> sec_queue;

int main(int argc, char** argv) {
    Info info;

    hospital = 5;
    second = 3;

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

    std::thread recvThread(receiverThread);

    while (isActive) {
        switch (STATE) {
            case 1: // Process report willingness to participate in a fight.
                {
                info.id = rank;
                info.time = CLOCK;
                queue.push_back(info);
                sort(queue.begin(), queue.end(), sortByTime);
                for(int i=0; i<size; ++i) {
                    if (i != rank) {
                        buf = CLOCK;
                        CLOCK = send_msg(buf, i, READY);
                    }
                }
                }
                break;

            case 2: // Process is looking for an opponent
                {
                position = findPosition(&queue, rank);

                if (position == -1 || position % 2 == 1)
                    break;
                
                if (position % 2 == 0) {
                    info = queue.at(position+1);
                    if (info.id == -1)
                        break;
                    buf = CLOCK;
                    CLOCK = send_msg(buf, info.id, FIGHT);
                    STATE = 0;
                    position = -1;
                }
                }

                break;

            case 3: // Process and their opponent are looking for a second.
                {
                for(int i=0; i<size; ++i) {
                        if (i != rank) {
                            buf = CLOCK;
                            CLOCK = send_msg(buf, i, SECOND);
                        }
                    }
                    STATE = 0;
                    wait_for_resource = 1;
                }
                break;

            case 4: // Process and their opponent get a second.
                STATE = 5;
                break;
                
            case 5: // Fight takes place and the second is being terminated.
                {
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
                        CLOCK = send_msg(buf, i, NOSECOND);
                    }
                }
                
                wait_for_resource = 0;

                while (sec_queue.size() > 0) {
                    sec_queue.erase(sec_queue.begin());
                    buf = CLOCK;
                    CLOCK = send_msg(buf, info.id, OKSECOND);
                }

                for (int i=0; i<queue.size(); i++) {
                    buf = CLOCK;
                    CLOCK = send_msg(buf, info.id, OK);
                }

                // forget opponent
                opponent = -1;
                }
                break;

            case 6: // Process which lost is looking for a hospital.
                {
                for(int i=0; i<size; ++i) {
                    if (i != rank) {
                        buf = CLOCK;
                        CLOCK = send_msg(buf, i, HOSPITAL);
                    }
                }
                STATE = 0;
                wait_for_resource = 2;
                }
                break;

            case 7: // Process is staying in a hospital.
                sleep(rand()%10+1);
                STATE = 8;
                break;

            case 8: // Process is leaving hospital.
                {
                for(int i=0; i<size; ++i) {
                    if (i != rank) {
                        buf = CLOCK;
                        CLOCK = send_msg(buf, i, NOHOSPITAL);
                    }
                }
                
                wait_for_resource = 0;
                STATE = 1;

                while (hsp_queue.size() > 0) {
                    hsp_queue.erase(hsp_queue.begin());
                    buf = CLOCK;
                    CLOCK = send_msg(buf, info.id, OKHOSPITAL);
                }
                }
                break;

            case 0: // Process is waiting.
                break;

            default:
                break;
        }
    
    }
    
    MPI_Finalize();
}