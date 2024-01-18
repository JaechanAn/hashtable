#include "queue.h"

#include <gtest/gtest.h>
#include <stdbool.h>
#include <unistd.h>

#define NUM_PRODUCER (32)
#define NUM_CONSUMER NUM_PRODUCER
#define NUM_THREADS (NUM_PRODUCER + NUM_CONSUMER)
#define NUM_ENQUEUE_PER_PRODUCER (100000)
#define NUM_DEQUEUE_PER_CONSUMER NUM_ENQUEUE_PER_PRODUCER

typedef struct ThreadArgs {
    int id;
    OperationQueue* queue;
    bool* flag_verification;
} ThreadArgs;

void* ProducerFunc(void* thd_args) {
    ThreadArgs* args = (ThreadArgs*)thd_args;

    int tid = args->id;
    OperationQueue* queue = args->queue;

    int key = NUM_ENQUEUE_PER_PRODUCER * tid;
    OperationType type = Insert;  // doesn't matter on queue operation test

    for (int i = 0; i < NUM_ENQUEUE_PER_PRODUCER; i++) {
        enqueue(queue, key, type);
        key++;
    }

    pthread_exit(NULL);
}

void* ConsumerFunc(void* thd_args) {
    ThreadArgs* args = (ThreadArgs*)thd_args;

    int tid = args->id;
    OperationQueue* queue = args->queue;
    bool* flag_verification = args->flag_verification;

    for (int i = 0; i < NUM_DEQUEUE_PER_CONSUMER; i++) {
        Operation op = dequeue(queue);
        flag_verification[op.key] = true;
    }

    pthread_exit(NULL);
}

/*
 * Test basic enqueue/dequeue.
 * 1. Produces and consumes keys from 0 ~ NUM_PRODUCER * NUM_ENQUEUE_PER_PRODUCER
 * 2. During consume, mark the flag for the key as true
 * 3. There should be no key left out with flag set to false
 *      - i.e, everything produces should be consumed
 */
TEST(QueueBasicTest, Correctness) {
    pthread_t threads[NUM_THREADS];
    ThreadArgs args[NUM_THREADS];

    bool flag_verification[NUM_PRODUCER * NUM_ENQUEUE_PER_PRODUCER];
    for (int i = 0; i < NUM_PRODUCER * NUM_ENQUEUE_PER_PRODUCER; ++i) {
        flag_verification[i] = false;
    }

    OperationQueue queue;
    init_queue(&queue);

    for (int i = 0; i < NUM_THREADS; i++) {
        args[i].id = i;
        args[i].queue = &queue;
        args[i].flag_verification = flag_verification;

        if (i < NUM_PRODUCER) {
            pthread_create(&threads[i], 0, ProducerFunc, (void**)&args[i]);
        } else {
            pthread_create(&threads[i], 0, ConsumerFunc, (void**)&args[i]);
        }
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    // verify that all produced are consumed
    for (int i = 0; i < NUM_PRODUCER * NUM_ENQUEUE_PER_PRODUCER; i++) {
        ASSERT_EQ(flag_verification[i], true);
    }
}
