#include "main.h"

std::mutex stateMutex;
std::condition_variable canGoFurther;

int size, rank;
int CLOCK, LAST_REQ, STATE;
int hospital, process;
int wait_for_resource = 0; // 0 - nothing, 1 = second, 2 - hospital
bool isActive = true;

std::vector<Info> queue;
std::vector<Info> hsp_queue;

int main(int argc, char** argv) {
    Info info;

    hospital = atoi(argv[1]);
    process = atoi(argv[2]);

    MPI_Status status;
    int buf;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    CLOCK = 0;
    LAST_REQ = 0;
    STATE = 1;

    std::thread recvThread(receiverThread);

    while (isActive) {
        
        stateMutex.lock();
        switch (STATE) {
            case 1: // Process report willingness to participate in a fight.
                {
                printf("Time %d, Rank %d, szukam przeciwnika\n", CLOCK, rank);
                LAST_REQ = CLOCK + 1;
                wait_for_resource = 1;
                STATE = 0;
                for(int i=0; i<size; ++i) {
                    if (i != rank) {
                        CLOCK++;
                        send_msg(CLOCK, i, READY);
                    }
                }
                printf("Time %d, Rank %d, still szukam przeciwnika\n", CLOCK, rank);
                }
                stateMutex.unlock();
                break;
                
            case 5: // Fight takes place and the second is being terminated.
                printf("Time %d, Rank %d, walcze z przeciwnikiem\n", CLOCK, rank);

                while (queue.size() > 0) {
                    printf("jestem\n");
                    info = queue.front();
                    queue.erase(queue.begin());
                    CLOCK++;
                    send_msg(CLOCK, info.id, OK);
                }

                wait_for_resource = 0;
                STATE = 1;
                stateMutex.unlock();
                printf("Time %d, Rank %d, zwalniam przeciwnika\n", CLOCK, rank);

                break;

            case 6: // Process which lost is looking for a hospital.
                // LAST_REQ = CLOCK + 1;
                // // stateMutex.lock();
                // wait_for_resource = 2;
                // STATE = 0;
                // // stateMutex.unlock();
                // for(int i=0; i<size; ++i) {
                //     if (i != rank) {
                //         CLOCK++;
                //         send_msg(CLOCK, i, HOSPITAL);
                //     }
                // }
                // // }
                stateMutex.unlock();
                break;

            case 7: // Process is staying in a hospital.
                // printf("Time %d, Rank %d, leczę się", CLOCK, rank);
                // sleep(rand()%10+1);
                // STATE = 8;
                stateMutex.unlock();
                break;

            case 8: // Process is leaving hospital.
                // printf("Time %d, Rank %d, zwalniam szpital", CLOCK, rank);
                
                // wait_for_resource = 0;
                // STATE = 1;
                // while (hsp_queue.size() > 0) {
                //     info = hsp_queue.front();
                //     hsp_queue.erase(hsp_queue.begin());
                //     buf = CLOCK;
                //     CLOCK = send_msg(buf, info.id, OKHOSPITAL);
                // }
                stateMutex.unlock();

                break;

            case 0: // Process is waiting.
                stateMutex.unlock();
                break;

            default:
                stateMutex.unlock();
                break;
        }
    }
    
    MPI_Finalize();
}