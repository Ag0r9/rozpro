#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include "queue_fun.h"

struct Queue* createQueue(unsigned capacity)
{
    struct Queue* queue = (struct Queue*)malloc(
        sizeof(struct Queue));
    queue->capacity = capacity;
    queue->front = queue->size = 0;
 
    queue->rear = capacity - 1;
    queue->array = (int*)malloc(
        queue->capacity * sizeof(struct Info));
    return queue;
}

int isFull(struct Queue* queue)
{
    return (queue->size == queue->capacity);
}

int isEmpty(struct Queue* queue)
{
    return (queue->size == 0);
}

void enqueue(struct Queue* queue, struct Info item)
{
    if (isFull(queue))
        return;
    queue->rear = (queue->rear + 1)
                  % queue->capacity;
    queue->array[queue->rear] = item;
    queue->size = queue->size + 1;
    printf("%d enqueued to queue\n", item);
}

struct Info dequeue(struct Queue* queue)
{
    if (isEmpty(queue)) {
        struct Info info;
        info.id = -1;
        info.time = -1;
        return info;
    }
    struct Info item = queue->array[queue->front];
    queue->front = (queue->front + 1)
                   % queue->capacity;
    queue->size = queue->size - 1;
    return item;
}

struct Info front(struct Queue* queue)
{
    if (isEmpty(queue)) {
        struct Info info;
        info.id = -1;
        info.time = -1;
        return info;
    }
    return queue->array[queue->front];
}

struct Info rear(struct Queue* queue)
{
    if (isEmpty(queue)) {
        struct Info info;
        info.id = -1;
        info.time = -1;
        return info;
    }
    return queue->array[queue->rear];
}