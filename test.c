#include <stdio.h>
#include <stdlib.h>
#include "queue_fun.h"

int main() {
    struct Info info;
    struct Queue* queue;

    queue = createQueue(3);

    int empty = isEmpty(queue);
    printf("Is empty? - %d\n", empty);

    for (int i = 0; i<3; ++i) {
        info.id = i;
        info.time = i;

        enqueue(queue, info);
    }

    empty = isFull(queue);
    printf("Is full? - %d\n", empty);

    info = front(queue);
    printf("Front item. Id - %d, time - %d", info.id, info.time);

    info = rear(queue);
    printf("Rear item. Id - %d, time - %d", info.id, info.time);

    dequeue(queue);
    info = front(queue);
    printf("New front item. Id - %d, time - %d", info.id, info.time);

    return 0;
}