#include <stdio.h>
#include <stdlib.h>

#include "queue_fun.h"

int main(int argc, char** argv) {
    struct Info aa;
    struct Queue* queue;

    queue = createQueue(4);
    
    aa.id = 0;
    aa.time = 1;
    enqueue(queue, aa);

    aa.id = 1;
    aa.time = 2;
    enqueue(queue, aa);

    aa.id = 2;
    aa.time = 999;
    enqueue(queue, aa);

    aa.id = 3;
    aa.time = 13;
    enqueue(queue, aa);

    sort(queue);

    printf("Koniec\n");
    for(int i=0; i<4; i++) {
        aa = dequeue(queue);
        printf("time %d, id %d\n", aa.time, aa.id);
    }
}