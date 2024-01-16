#include <stdio.h>
#include <stdlib.h>

#include "hashtable.h"

int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <hashtable_size>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int hashtable_size = atoi(argv[1]);

    HashTable* table = create_hashtable(hashtable_size);
    if (table == NULL) {
        fprintf(stderr, "Failed to create hash table with %d buckets.", hashtable_size);
    }

    for (int i = 0; i < 15; ++i) {
        insert_hashtable(table, i);
    }

    print_hashtable(table);

    int lookup_keys[3] = {1, 5, 18};
    for (int i = 0; i < 3; ++i) {
        Node* node = lookup_hashtable(table, lookup_keys[i]);
        if (node != NULL) {
            printf("Found key {%d}!\n", lookup_keys[i]);
        } else {
            printf("Did not find key {%d}!\n", lookup_keys[i]);
        }

        int deleted = delete_hashtable(table, lookup_keys[i]);
        if (deleted == 0) {
            printf("Deleted key {%d}!\n", lookup_keys[i]);
        } else {
            printf("Failed to delete key {%d}!\n", lookup_keys[i]);
        }
    }

    print_hashtable(table);

    int freed = free_hashtable(table);
    if (freed != 0) {
        fprintf(stderr, "Failed to free hash table.");
    }

    return EXIT_SUCCESS;
}
