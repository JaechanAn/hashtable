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
int left_over;  // last worker will signal the main thread, should set it to num_threads

typedef struct ThreadArgs {
    int id;
    HashTable* table;
    OperationQueue* queue;
    int num_ops;
    bool is_ready;
} ThreadArgs;

// Works as workload consumer
void* thread_func(void* thd_args) {
    ThreadArgs* args = (ThreadArgs*)thd_args;

    int tid = args->id;
    HashTable* table = args->table;
    OperationQueue* queue = args->queue;
    int num_ops = args->num_ops;

    // Wait until all workers are generated
    pthread_mutex_lock(&worker_mutex);
    args->is_ready = true;  // This is necessary since main thread might surpass the worker thread sleep
    pthread_cond_wait(&worker_cond, &worker_mutex);
    pthread_mutex_unlock(&worker_mutex);

    for (int i = 0; i < num_ops; i++) {
        Operation op;
        op = dequeue(queue);

        // printf("[Server %d] type: %d, key: %d\n", tid, (int)op.type, op.key);
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

    int order = __sync_sub_and_fetch(&left_over, 1);
    if (order == 0) {  // last thread exiting should wakeup the main thread
        pthread_mutex_lock(&main_mutex);
        pthread_cond_signal(&main_cond);
        pthread_mutex_unlock(&main_mutex);
    }

    pthread_exit(NULL);
}

int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <hashtable_size>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int hashtable_size = atoi(argv[1]);
    if (hashtable_size <= 0) {
        fprintf(stderr, "<hashtable_size> must be an integer greater than 0.\n");
        exit(EXIT_FAILURE);
    }

    // Initialize shared memory
    SharedMem* area = (SharedMem*)shm_create();
    if (area == NULL) {
        fprintf(stderr, "Failed to initialize shared memory.\n");
        exit(EXIT_FAILURE);
    }
    fprintf(stdout, "Initialized shared memory of size: %ld\n", sizeof(*area));

    shm_init(area);

    // Setup operation queue for client/server communication
    init_queue(&area->queue);

    HashTable* table = hashtable_create(hashtable_size);
    if (table == NULL) {
        fprintf(stderr, "Failed to create hash table with %d buckets.", hashtable_size);
    }

    area->server_is_ready = true;

    while (!area->client_is_ready) {
    }

    pthread_t threads[area->num_threads];
    ThreadArgs args[area->num_threads];

    left_over = area->num_threads;

    for (int i = 0; i < area->num_threads; i++) {
        args[i].id = i;
        args[i].table = table;
        args[i].queue = &area->queue;
        args[i].num_ops = area->num_ops_per_thread;
        args[i].is_ready = false;

        pthread_create(&threads[i], 0, thread_func, (void**)&args[i]);

        // Wait for the worker thread to fall asleep. Using spin wait seems enough.
        while (!args[i].is_ready) {
            pthread_yield();  // yielding might be a bad idea since the term will be short
        }
    }

    // Wake the worker threads waiting on the condition variable
    pthread_mutex_lock(&worker_mutex);

    // Acquire main mutex before waking up worker threads to prevent all workers from
    // terminating before main thread asleep.
    pthread_mutex_lock(&main_mutex);

    // Awake worker threads
    pthread_cond_broadcast(&worker_cond);
    pthread_mutex_unlock(&worker_mutex);

    // Main thread asleep
    pthread_cond_wait(&main_cond, &main_mutex);
    pthread_mutex_unlock(&main_mutex);

    for (int i = 0; i < area->num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    int freed = hashtable_free(table);
    if (freed != 0) {
        fprintf(stderr, "Failed to free hash table.");
    }

    shm_free(area);

    return EXIT_SUCCESS;
}
