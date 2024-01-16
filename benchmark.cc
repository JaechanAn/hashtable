#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "hashtable.h"

#define HASHTABLE_SIZE (1000)         // reduce this to increase contention among buckets
#define NUM_OPS_PER_THREAD (1000000)  // 1M ops per thread

enum WorkloadType { Insert = 0, Delete = 1, Lookup = 2 };

typedef struct ThreadArgs {
    int id;
    WorkloadType type;
    int num_ops;
    HashTable* table;
} ThreadArgs;

double elapsed_seconds(struct timespec* begin, struct timespec* end) {
    return (end->tv_nsec - begin->tv_nsec) / 1000000000.0 + (end->tv_sec - begin->tv_sec);
}

void* thread_func(void* thd_args);

int main(int argc, char** argv) {
    srand(time(NULL));

    long ncores = sysconf(_SC_NPROCESSORS_ONLN);
    int num_threads = ncores * 3;  // ncores thread per each operation {insert, delete, lookup}
    printf("Performing benchmark on machine with %ld cores.\n", ncores);

    HashTable* table = hashtable_create(HASHTABLE_SIZE);
    if (table == NULL) {
        fprintf(stderr, "Failed to create hash table with %d buckets.", HASHTABLE_SIZE);
    }

    struct timespec begin, end;
    clock_gettime(CLOCK_MONOTONIC_RAW, &begin);

    pthread_t threads[num_threads];
    ThreadArgs args[num_threads];

    for (int i = 0; i < num_threads; i++) {
        args[i].id = i;
        args[i].num_ops = NUM_OPS_PER_THREAD;
        args[i].table = table;
        args[i].type = (WorkloadType)(i);  // Must match enum WorkloadType values
        pthread_create(&threads[i], NULL, thread_func, (void**)&args[i]);
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    clock_gettime(CLOCK_MONOTONIC_RAW, &end);
    fprintf(stdout, "It took %.3f seconds", elapsed_seconds(&begin, &end));

    int freed = hashtable_free(table);
    if (freed != 0) {
        fprintf(stderr, "Failed to free hash table.");
    }

    return EXIT_SUCCESS;
}

void* thread_func(void* thd_args) {
    ThreadArgs* args = (ThreadArgs*)thd_args;
    int id = args->id;
    WorkloadType type = args->type;
    int num_ops = args->num_ops;
    HashTable* table = args->table;

    for (int i = 0; i < num_ops; ++i) {
        int key = rand();  // TODO: skewed workload test
        switch (type) {
            case Insert:
                hashtable_insert(table, key);  // result doesn't matter
                break;
            case Delete:
                hashtable_delete(table, key);  // result doesn't matter
                break;
            case Lookup:
                hashtable_lookup(table, key);  // result doesn't matter
                break;
        }
    }

    pthread_exit(NULL);
}
