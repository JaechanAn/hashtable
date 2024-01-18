#ifndef QUEUE_H_
#define QUEUE_H_

#include <stdint.h>

#define QUEUE_SIZE (1024)

enum OperationType { Undefined = -1, Insert = 0, Delete = 1, Lookup = 2 };

typedef struct Operation {
    int key;
    OperationType type;
    uint64_t flag;  // for fairness
} Operation;

typedef struct OperationQueue {
    Operation instructions[QUEUE_SIZE];
    int front;
    int rear;
} OperationQueue;

void init_queue(OperationQueue* queue);
void enqueue(OperationQueue* queue, int key, OperationType type);
Operation dequeue(OperationQueue* queue);

#endif /* QUEUE_H_ */
