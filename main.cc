#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "hashtable.h"

int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <hashtable_size>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    long ncores = sysconf(_SC_NPROCESSORS_ONLN);
    printf("ncores: %ld\n", ncores);

    int hashtable_size = atoi(argv[1]);

    HashTable* table = hashtable_create(hashtable_size);
    if (table == NULL) {
        fprintf(stderr, "Failed to create hash table with %d buckets.", hashtable_size);
    }

    for (int i = 0; i < 15; ++i) {
        hashtable_insert(table, i);
    }

    hashtable_print(table);

    int lookup_keys[3] = {1, 5, 18};
    for (int i = 0; i < 3; ++i) {
        Node* node = hashtable_lookup(table, lookup_keys[i]);
        if (node != NULL) {
            printf("Found key {%d}!\n", lookup_keys[i]);
        } else {
            printf("Did not find key {%d}!\n", lookup_keys[i]);
        }

        int deleted = hashtable_delete(table, lookup_keys[i]);
        if (deleted == 0) {
            printf("Deleted key {%d}!\n", lookup_keys[i]);
        } else {
            printf("Failed to delete key {%d}!\n", lookup_keys[i]);
        }
    }

    hashtable_print(table);

    int freed = hashtable_free(table);
    if (freed != 0) {
        fprintf(stderr, "Failed to free hash table.");
    }

    return EXIT_SUCCESS;
}
