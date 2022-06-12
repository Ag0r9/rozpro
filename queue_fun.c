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
    queue->array = (struct Info*)malloc(
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

void enqueue(struct Queue* queue, struct Info item)
{
    if (isFull(queue))
        return;

    queue->rear = (queue->rear + 1)
                  % queue->capacity;
    queue->array[queue->rear] = item;
    queue->size = queue->size + 1;
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
    queue->front = (queue->front + 1) % queue->capacity;
    queue->size = queue->size - 1;
    return item;
}

void sort(struct Queue* queue)
{
    struct Info a, b;
    struct Info pom;
    int i=0, j, check;
	int l = queue->capacity;
	while(l--) {
		if(isEmpty(queue))
			break;
		a = dequeue(queue);
		if(isEmpty(queue)) {
            enqueue(queue, a);
            break;
        }
		b = dequeue(queue);
		j = l-i-1;
		while(j--) {
            pom = front(queue);
			if(a.time<b.time) {
				if(pom.time>a.time) {
					enqueue(queue, a);
					a = dequeue(queue); 
				} else
					break;
			} else {
				if(pom.time>b.time) {
					enqueue(queue, b);
					b = dequeue(queue);
				} else
					break ;
			}
		}

		if(a.time>b.time) {
			enqueue(queue, b);
			enqueue(queue, a);
		} else {
			enqueue(queue, a);
			enqueue(queue, b);
		}
		i++;
	}
}

struct Info findFellowBehindMe(struct Queue* queue, int id) {
    struct Info info;
    struct Info results;
    results.id = -1;
    results.time = -1;
    int is_found = 0;
    int length = queue->size;

    if (isEmpty(queue))
        return results;

    for (int i=0; i<length; ++i) {
        info = dequeue(queue);
        if (is_found == 1) {
            results.id = info.id;
            results.time = info.time;
            is_found = -1;
        }
        
        if (info.id == id && is_found == 0) {
            is_found = 1;
        }

        enqueue(queue, info);
    }
    return results;
}

int findPosition(struct Queue* queue, int id) {
    struct Info info;
    struct Info results;
    int answer = -1;
    int length = queue->size;

    if (isEmpty(queue))
        return answer;

    for (int i=0; i<length; ++i) {
        info = dequeue(queue);

        if (info.id == id && answer == -1)
            answer = i;

        enqueue(queue, info);
    }
    return answer;
}

struct Info deleteById(struct Queue* queue, int id) {
    struct Info info;
    struct Info result;

    int found = 0;
    
    result.id = -1;
    result.time = -1;

    if (isEmpty(queue)) {
        return result;
    }

    int size = queue->size;
    for (int i=0; i<size; ++i) {
        info = dequeue(queue);
        if (info.id == id && found == 0) {
            result.id = info.id;
            result.time = result.time;
            found = 1;
            continue;
        }
        enqueue(queue, info);
    }
    
    return result;
}