#include "main.h"

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

std::mutex stateMutex;
std::condition_variable canGoFurther;

int size, rank;
int CLOCK, STATE;
int hospital, process;
int wait_for_resource = 0; // 0 - nothing, 1 = second, 2 - hospital
int is_approved, coded_msg;
bool isActive = true;

std::vector<Info> queue;
std::vector<Info> hsp_queue;

int main(int argc, char** argv) {
    Info info;

    hospital = 4;
    process = 3;

    MPI_Status status;
    int buf;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    CLOCK = 0;
    STATE = 1;

    std::thread recvThread(receiverThread);

    while (isActive) {
        switch (STATE) {
            case 1: // Process report willingness to participate in a fight.
                {
                printf("Time %d, Rank %d, szukam przeciwnika\n", CLOCK, rank);
                for(int i=0; i<size; ++i) {
                    if (i != rank) {
                        buf = CLOCK;
                        CLOCK = send_msg(buf, i, READY);
                    }
                }
                STATE = 0;
                wait_for_resource = 1;
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
                // printf("Time %d, Rank %d, szukam sekundanta\n", CLOCK, rank);
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
                // printf("Time %d, Rank %d, mam sekundanta\n", CLOCK, rank);
                STATE = 5;
                break;
                
            case 5: // Fight takes place and the second is being terminated.
                {
                wait_for_resource = 0;
                STATE = 6;

                while (queue.size() > 0) {
                    info = queue.front();
                    queue.erase(queue.begin());
                    buf = CLOCK;
                    CLOCK = send_msg(buf, info.id, OK);
                }
                }
                break;

            case 6: // Process which lost is looking for a hospital.
                {
                printf("Time %d, Rank %d, szukam szpitala\n", CLOCK, rank);
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
                printf("Time %d, Rank %d, leczę się\n", CLOCK, rank);
                sleep(rand()%10+1);
                STATE = 8;
                break;

            case 8: // Process is leaving hospital.
                printf("Time %d, Rank %d, zwalniam szpital\n", CLOCK, rank);
                
                wait_for_resource = 0;
                STATE = 1;

                while (hsp_queue.size() > 0) {
                    info = hsp_queue.front();
                    hsp_queue.erase(hsp_queue.begin());
                    buf = CLOCK;
                    CLOCK = send_msg(buf, info.id, OKHOSPITAL);
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