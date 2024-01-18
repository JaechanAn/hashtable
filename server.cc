#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "hashtable.h"
#include "shm.h"

int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <hashtable_size>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int hashtable_size = atoi(argv[1]);
    if (hashtable_size <= 0) {
        fprintf(stderr, "<hashtable_size> must be greater than 0\n");
        exit(EXIT_FAILURE);
    }

    // Initialize shared memory
    SharedMem* area = (SharedMem*)shm_init();
    fprintf(stdout, "Initialized shared memory of size: %ld\n", sizeof(*area));

    // Setup operation queue for client/server communication
    init_queue(&area->queue);

    area->is_ready = true;

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

    shm_free(area);

    return EXIT_SUCCESS;
}
