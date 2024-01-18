#include "queue.h"

#include <pthread.h>

void init_queue(OperationQueue* queue) {
    queue->front = 0;
    queue->rear = 0;
    for (int i = 0; i < QUEUE_SIZE; ++i) {
        queue->instructions[i].key = -1;
        queue->instructions[i].flag = 0;
        queue->instructions[i].type = Undefined;
    }
}

void enqueue(OperationQueue* queue, int key, OperationType type) {
    uint64_t seq = __sync_fetch_and_add(&queue->rear, 1);
    int slot_idx = seq % QUEUE_SIZE;
    uint64_t round = seq / QUEUE_SIZE;

    while (1) {
        uint64_t flag = queue->instructions[slot_idx].flag;
        if (flag % 2 == 1) {  // queue is full
            pthread_yield();
        } else {
            if (flag / 2 == round) {  // for fairness
                queue->instructions[slot_idx].key = key;
                queue->instructions[slot_idx].type = type;
                __sync_synchronize();
                queue->instructions[slot_idx].flag++;
                break;
            } else {
                pthread_yield();
            }
        }
    }
}

Operation dequeue(OperationQueue* queue) {
    uint64_t seq = __sync_fetch_and_add(&queue->front, 1);
    int slot_idx = seq % QUEUE_SIZE;
    uint64_t round = seq / QUEUE_SIZE;
    Operation ret;

    while (1) {
        uint64_t flag = queue->instructions[slot_idx].flag;
        if (flag % 2 == 0) {  // queue is empty
            pthread_yield();
        } else {
            if (flag / 2 == round) {  // for fairness
                ret.key = queue->instructions[slot_idx].key;
                ret.type = queue->instructions[slot_idx].type;
                __sync_synchronize();
                queue->instructions[slot_idx].flag++;
                break;
            } else {
                pthread_yield();
            }
        }
    }

    return ret;
}
