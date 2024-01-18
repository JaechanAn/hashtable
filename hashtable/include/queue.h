/**
 * NOTE: Implementation of a concurrent queue with bounded size. The queue
 * uses a mechanism of spin wait, but yields when not its turn. If one of the
 * producer or consumer is extremely slower than the other, it may be a better
 * idea to use sleep wait using conditional variable. For now, we expect the
 * producer and consumer to be actively participating.
 */

#ifndef QUEUE_H_
#define QUEUE_H_

#include <stdbool.h>
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
    bool is_ready;
} OperationQueue;

void init_queue(OperationQueue* queue);

void enqueue(OperationQueue* queue, int key, OperationType type);

Operation dequeue(OperationQueue* queue);

// NOTE: If this function returns true, it means that the queue is empty for this moment.
// If used properly with the should_terminate variable of struct SharedMem, we can detect
// if the queue is actually empty and will be empty (i.e., no more enqueue). Remember that
// this is a workaround and there might be a better way to determine the termination point.
bool queue_is_empty(OperationQueue* queue);

#endif /* QUEUE_H_ */
