#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

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
    OperationQueue* queue;
    int num_ops;
    bool is_ready;
} ThreadArgs;

// Works as workload producer
void* thread_func(void* thd_args) {
    ThreadArgs* args = (ThreadArgs*)thd_args;

    int tid = args->id;
    OperationQueue* queue = args->queue;
    int num_ops = args->num_ops;

    // Wait until all workers are generated
    pthread_mutex_lock(&worker_mutex);
    args->is_ready = true;  // This is necessary since main thread might surpass the worker thread sleep
    pthread_cond_wait(&worker_cond, &worker_mutex);  // Worker threads awoken while main_mutex held by main thread
    pthread_mutex_unlock(&worker_mutex);

    for (int i = 0; i < num_ops; i++) {
        int key = rand();
        OperationType type = (OperationType)(i % 3);  // Must match enum OperationType values
        // printf("[Client %d] type: %d, key: %d\n", tid, (int)type, key);
        enqueue(queue, key, type);
    }

    int order = __sync_sub_and_fetch(&left_over, 1);
    if (order == 0) {  // last thread exiting should wakeup the main thread
        // if success, main_mutex should be released, meaning that main thread is asleep
        pthread_mutex_lock(&main_mutex);
        pthread_cond_signal(&main_cond);
        pthread_mutex_unlock(&main_mutex);
    }

    pthread_exit(NULL);
}

int main(int argc, char** argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <num_threads> <num_ops_per_thread>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int num_threads = atoi(argv[1]);
    int num_ops_per_thread = atoi(argv[2]);
    if (num_threads <= 0 || num_ops_per_thread <= 0) {
        fprintf(stderr, "<num_threads> and <num_ops_per_thread> must be an integer greater than 0.\n");
        exit(EXIT_FAILURE);
    }

    srand(time(NULL));

    SharedMem* area = (SharedMem*)shm_attach();
    if (area == NULL) {
        fprintf(stderr, "Failed to load shared memory. Please make sure that the server is running.\n");
        exit(EXIT_FAILURE);
    }

    fprintf(stdout, "Waiting for server...\n");

    while (!area->server_is_ready) {
        usleep(100000);  // sleep 100 ms
    }

    fprintf(stdout, "Server is ready, preparing client.\n");

    area->num_threads = num_threads;
    area->num_ops_per_thread = num_ops_per_thread;

    area->client_is_ready = true;

    fprintf(stdout, "Client is ready! Sending operations to server.\n");

    pthread_t threads[num_threads];
    ThreadArgs args[num_threads];

    left_over = num_threads;

    for (int i = 0; i < num_threads; i++) {
        args[i].id = i;
        args[i].queue = &area->queue;
        args[i].num_ops = num_ops_per_thread;
        args[i].is_ready = false;

        pthread_create(&threads[i], 0, thread_func, (void**)&args[i]);

        // Wait for the worker thread to fall asleep. Using spin wait seems enough
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

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    return EXIT_SUCCESS;
}
