#include "main.h"

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_RESET   "\x1b[0m"

std::mutex stateMutex;
std::condition_variable canGoFurther;

int size, rank;
int CLOCK, LAST_REQ, STATE;
int hospital;
int wait_for_resource; // 0 - nothing, 2 - hospital
bool isActive = true;

std::vector<Info> hsp_queue;

int main(int argc, char** argv) {
    Info info;

    srandom(time(NULL) + getpid() + rank);

    hospital = atoi(argv[1]);

    MPI_Status status;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    CLOCK = 0;
    LAST_REQ = 0;

    std::thread recvThread(receiverThread);

    while (isActive) {
        printf(ANSI_COLOR_BLUE "Time %d, Rank %d, walcze z przeciwnikiem ", CLOCK, rank);
        stateMutex.lock();
        wait_for_resource = 0;
        STATE = 1;
        stateMutex.unlock();
        int fight_time = rand()%5+1; 
        printf("przez %d\n", fight_time);
        sleep(fight_time);


        printf(ANSI_COLOR_YELLOW  "Time %d, Rank %d, ubiegam się o szpital\n", CLOCK, rank);
        stateMutex.lock();
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

        std::unique_lock<std::mutex> lk(stateMutex);
        canGoFurther.wait(lk);

        printf(ANSI_COLOR_RED "Time %d, Rank %d, leczę się\n", CLOCK, rank);
        // stateMutex.lock();
        STATE = 8;
        // stateMutex.unlock();
        // printf("AAA %d\n", rank);
        sleep(rand()%10+1);

        // printf("BBB %d\n", rank);

        printf(ANSI_COLOR_GREEN "Time %d, Rank %d, zwalniam szpital\n", CLOCK, rank);
                
        while (hsp_queue.size() > 0) {
            info = hsp_queue.front();
            hsp_queue.erase(hsp_queue.begin());
            CLOCK++;
            send_msg(CLOCK, info.id, OKHOSPITAL);
        }
    }
    
    MPI_Finalize();
}