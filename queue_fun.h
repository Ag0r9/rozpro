// C program for array implementation of queue
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

struct Info {
    int time;
    int id;
};


struct Queue {
    int front;
    int rear;
    int size;
    unsigned capacity;
    struct Info* array;
};


struct Queue* createQueue(unsigned capacity);
int isFull(struct Queue* queue);
int isEmpty(struct Queue* queue);
void enqueue(struct Queue* queue, struct Info item);
struct Info dequeue(struct Queue* queue);
struct Info front(struct Queue* queue);
struct Info rear(struct Queue* queue);
void sort(struct Queue* Queue);
