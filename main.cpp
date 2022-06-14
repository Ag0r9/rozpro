#include "main.h"

std::mutex stateMutex;
std::condition_variable canGoFurther;

int size, rank;
int CLOCK, LAST_REQ, STATE;
int hospital, process;
int wait_for_resource = 0; // 0 - nothing, 1 - opponent, 2 - hospital
bool isActive = true;

std::vector<Info> queue;
std::vector<Info> hsp_queue;

int main(int argc, char** argv) {
    Info info;

    srandom(time(NULL)+getpid()+rank);

    hospital = atoi(argv[1]);
    process = atoi(argv[2]);

    MPI_Status status;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    CLOCK = 0;
    LAST_REQ = 0;
    STATE = 1;

    std::thread recvThread(receiverThread);

    while (isActive) {
        
        stateMutex.lock();

        printf("Time %d, Rank %d, walcze z przeciwnikiem\n", CLOCK, rank);
                int t =rand()%5+1; 
                printf("przez %d\n", t);
                sleep(t);
               
                //STATE = 6;

            // case 6: // Process which lost is looking for a hospital.
                printf("Time %d, Rank %d, ubiegam się o szpital\n", CLOCK, rank);
                LAST_REQ = CLOCK + 1;
                wait_for_resource = 2;
                STATE = 0;

                for(int i=0; i<size; ++i) {
                    if (i != rank) {
                        CLOCK++;
                        send_msg(CLOCK, i, HOSPITAL);
                    }
                }
                stateMutex.unlock();
                // break;

                std::unique_lock<std::mutex> lk(stateMutex);
                canGoFurther.wait(lk);

            // case 7: // Process is staying in a hospital.
                printf("Time %d, Rank %d, leczę się\n", CLOCK, rank);
                sleep(rand()%10+1);
                STATE = 8;
                stateMutex.unlock();
                // break;

            // case 8: // Process is leaving hospital.
                printf("Time %d, Rank %d, zwalniam szpital\n", CLOCK, rank);
                
                while (hsp_queue.size() > 0) {
                    info = hsp_queue.front();
                    hsp_queue.erase(hsp_queue.begin());
                    CLOCK++;
                    send_msg(CLOCK, info.id, OKHOSPITAL);
                }

                wait_for_resource = 0;
                STATE = 1;
                stateMutex.unlock();

                // break;

            // case 0: // Process is waiting.
                stateMutex.unlock();
                // break;

            // default:
                stateMutex.unlock();
                // break;
        // }
    }
    
    MPI_Finalize();
}