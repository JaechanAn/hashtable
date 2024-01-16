#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "hashtable.h"
#include "shm.h"

typedef struct {
    void* area;
    size_t size;
} SharedMem;

int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <hashtable_size>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    long ncores = sysconf(_SC_NPROCESSORS_ONLN);
    printf("ncores: %ld\n", ncores);

    int hashtable_size = atoi(argv[1]);

    // Initialize shared memory depending on the hash table size
    size_t shm_size = hashtable_estimate_size(hashtable_size);
    SharedMem* shm = (SharedMem*)shm_create(shm_size);

    HashTable* table = hashtable_create(hashtable_size);
    if (table == NULL) {
        fprintf(stderr, "Failed to create hash table with %d buckets.", hashtable_size);
    }

    /**
     * TODO
     */

    int freed = hashtable_free(table);
    if (freed != 0) {
        fprintf(stderr, "Failed to free hash table.");
    }

    shm_free(shm, shm_size);

    return EXIT_SUCCESS;
}
