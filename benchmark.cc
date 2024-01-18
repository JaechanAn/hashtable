#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "hashtable.h"
#include "queue.h"

typedef struct ThreadArgs {
    int id;
    OperationType type;
    int num_ops;
    HashTable* table;

    double accumulated_insert_latency;
    double accumulated_delete_latency;
    double accumulated_lookup_latency;
} ThreadArgs;

double elapsed_ms(struct timespec* begin, struct timespec* end) {
    return (end->tv_nsec - begin->tv_nsec) / 1000000.0 + (end->tv_sec - begin->tv_sec) * 1000;
}

void* thread_func(void* thd_args);

int main(int argc, char** argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <hashtable_size> <num_ops_per_thread>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int hashtable_size = atoi(argv[1]);
    int num_ops_per_thread = atoi(argv[2]);
    if (hashtable_size <= 0 || num_ops_per_thread <= 0) {
        fprintf(stderr, "<hashtable_size> and <num_ops_per_thread> must be an integer greater than 0.\n");
        exit(EXIT_FAILURE);
    }

    srand(time(NULL));

    long ncores = sysconf(_SC_NPROCESSORS_ONLN);
    int num_threads = ncores * 3;  // ncores thread per each operation {insert, delete, lookup}
    printf("Performing benchmark on machine with %ld cores, %d threads.\n", ncores, num_threads);

    HashTable* table = hashtable_create(hashtable_size);
    if (table == NULL) {
        fprintf(stderr, "Failed to create hash table with %d buckets.", hashtable_size);
    }

    struct timespec begin, end;
    clock_gettime(CLOCK_MONOTONIC_RAW, &begin);

    pthread_t threads[num_threads];
    ThreadArgs args[num_threads];

    for (int i = 0; i < num_threads; i++) {
        args[i].id = i;
        args[i].num_ops = num_ops_per_thread;
        args[i].table = table;
        args[i].type = (OperationType)(i % 3);  // Must match enum OperationType values
        args[i].accumulated_insert_latency = 0;
        args[i].accumulated_delete_latency = 0;
        args[i].accumulated_lookup_latency = 0;
        pthread_create(&threads[i], NULL, thread_func, (void**)&args[i]);
    }

    /**
     * TODO: Wait for all threads to be created before starting their job by using conditional variable.
     */

    double total_insert_latency = 0.0;
    double total_delete_latency = 0.0;
    double total_lookup_latency = 0.0;
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);

        total_insert_latency += args[i].accumulated_insert_latency;
        total_delete_latency += args[i].accumulated_delete_latency;
        total_lookup_latency += args[i].accumulated_lookup_latency;
    }

    clock_gettime(CLOCK_MONOTONIC_RAW, &end);
    fprintf(stdout, "Total operation time (ms): %.3f\n", elapsed_ms(&begin, &end));

    printf("Average time per ops (ms):\n");
    printf("\tinsert: %f\n", total_insert_latency / num_ops_per_thread);
    printf("\tdelete: %f\n", total_delete_latency / num_ops_per_thread);
    printf("\tlookup: %f\n", total_lookup_latency / num_ops_per_thread);

    int freed = hashtable_free(table);
    if (freed != 0) {
        fprintf(stderr, "Failed to free hash table.");
    }

    return EXIT_SUCCESS;
}

void* thread_func(void* thd_args) {
    ThreadArgs* args = (ThreadArgs*)thd_args;

    int id = args->id;
    OperationType type = args->type;
    int num_ops = args->num_ops;
    HashTable* table = args->table;

    for (int i = 0; i < num_ops; ++i) {
        int key = rand();  // TODO: skewed workload test

        struct timespec begin, end;
        clock_gettime(CLOCK_MONOTONIC_RAW, &begin);

        switch (type) {
            case Insert:
                hashtable_insert(table, key);

                clock_gettime(CLOCK_MONOTONIC_RAW, &end);
                args->accumulated_insert_latency += elapsed_ms(&begin, &end);
                break;
            case Delete:
                hashtable_delete(table, key);

                clock_gettime(CLOCK_MONOTONIC_RAW, &end);
                args->accumulated_delete_latency += elapsed_ms(&begin, &end);
                break;
            case Lookup:
                hashtable_lookup(table, key);

                clock_gettime(CLOCK_MONOTONIC_RAW, &end);
                args->accumulated_lookup_latency += elapsed_ms(&begin, &end);
                break;
        }
    }

    pthread_exit(NULL);
}
