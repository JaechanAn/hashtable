#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "hashtable.h"
#include "queue.h"
#include "shm.h"

// For controlling the worker threads
pthread_cond_t worker_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t worker_mutex = PTHREAD_MUTEX_INITIALIZER;

// For controlling the main thread
pthread_cond_t main_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t main_mutex = PTHREAD_MUTEX_INITIALIZER;
int not_done;  // last worker will signal the main thread, should set it to num_threads

typedef struct ThreadArgs {
    int id;
    HashTable* table;
    OperationQueue* queue;
    bool* should_terminate;
    bool is_ready;
} ThreadArgs;

// Works as workload consumer
void* thread_func(void* thd_args) {
    ThreadArgs* args = (ThreadArgs*)thd_args;

    int tid = args->id;
    HashTable* table = args->table;
    OperationQueue* queue = args->queue;
    bool* should_terminate = args->should_terminate;

    // Wait until all workers are generated
    pthread_mutex_lock(&worker_mutex);
    args->is_ready = true;  // This is necessary since main thread might surpass the worker thread sleep
    pthread_cond_wait(&worker_cond, &worker_mutex);
    pthread_mutex_unlock(&worker_mutex);

    // If should_terminate is true (set by client), the client is done with producing all the operations.
    // The server still has to take care of unfinished operations within the queue.
    while (!(*should_terminate) || !queue_is_empty(queue)) {
        Operation op;
        op = dequeue(queue);

        printf("[Server %d] type: %d, key: %d\n", tid, (int)op.type, op.key);
        switch (op.type) {
            case Insert:
                hashtable_insert(table, op.key);
                break;
            case Delete:
                hashtable_delete(table, op.key);
                break;
            case Lookup:
                hashtable_lookup(table, op.key);
                break;
            default:
                assert(false);  // should never happen
        }
    }

    int order = __sync_sub_and_fetch(&not_done, 1);
    if (order == 0) {  // last thread exiting should wakeup the main thread
        pthread_mutex_lock(&main_mutex);
        pthread_cond_signal(&main_cond);
        pthread_mutex_unlock(&main_mutex);
    }

    pthread_exit(NULL);
}

int main(int argc, char** argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <num_threads> <hashtable_size>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int num_threads = atoi(argv[1]);
    int hashtable_size = atoi(argv[2]);
    if (num_threads <= 0 || hashtable_size <= 0) {
        fprintf(stderr, "<num_threads> and <hashtable_size> must be an integer greater than 0.\n");
        exit(EXIT_FAILURE);
    }

    // Initialize shared memory
    SharedMem* area = (SharedMem*)shm_init();
    if (area == NULL) {
        fprintf(stderr, "Failed to initialize shared memory.\n");
        exit(EXIT_FAILURE);
    }
    fprintf(stdout, "Initialized shared memory of size: %ld\n", sizeof(*area));

    // Setup operation queue for client/server communication
    init_queue(&area->queue);

    HashTable* table = hashtable_create(hashtable_size);
    if (table == NULL) {
        fprintf(stderr, "Failed to create hash table with %d buckets.", hashtable_size);
    }

    pthread_t threads[num_threads];
    ThreadArgs args[num_threads];

    not_done = num_threads;

    for (int i = 0; i < num_threads; i++) {
        args[i].id = i;
        args[i].table = table;
        args[i].queue = &area->queue;
        args[i].should_terminate = &area->should_terminate;
        args[i].is_ready = false;

        pthread_create(&threads[i], 0, thread_func, (void**)&args[i]);

        // Wait for the worker thread to fall asleep. Using spin wait seems enough.
        while (!args[i].is_ready) {
            pthread_yield();  // yielding might be a bad idea since the term will be short
        }
    }

    // Wake the worker threads waiting on the condition variable
    pthread_mutex_lock(&worker_mutex);
    pthread_cond_broadcast(&worker_cond);
    pthread_mutex_unlock(&worker_mutex);

    // Sleep wait for all workers to finish
    pthread_mutex_lock(&main_mutex);
    pthread_cond_wait(&main_cond, &main_mutex);
    pthread_mutex_unlock(&main_mutex);

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    int freed = hashtable_free(table);
    if (freed != 0) {
        fprintf(stderr, "Failed to free hash table.");
    }

    shm_free(area);

    return EXIT_SUCCESS;
}
