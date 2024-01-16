#ifndef HASHTABLE_H_
#define HASHTABLE_H_

#include <pthread.h>
#include <stddef.h>

// TODO: support dynamic chain length combined with shared memory
#define MAX_CHAIN_LENGTH (1000000)  // per bucket maximum collisions: 1M

typedef struct Node {
    int key;            // currently supports integer key only
    struct Node* next;  // next pointer for handling linked list style chaining
#ifdef FINE_GRAINED_LOCKING
    pthread_rwlock_t* lock;
#endif /* FINE_GRAINED_LOCKING */
} Node;

Node* init_node(void);

typedef struct HashTable {
    Node** buckets;  // represents the table buckets
    int size;        // represents the bucket size, not the number of items
#ifdef COARSE_GRAINED_LOCKING
    pthread_rwlock_t* bucket_locks;
#endif /* COARSE_GRAINED_LOCKING */
} HashTable;

/*
 * Hash table control functions
 */
size_t hashtable_estimate_size(size_t hashtable_size);

// Create a hash table of the given size.
// Must be called at the initialization process by the main thread.
HashTable* hashtable_create(int size);

// Free a hash table.
// Must be called at the termination process by the main thread.
int hashtable_free(HashTable* table);

// Returns the hash value representing the bucket index;
int hash_func(int key, int size);

// Insert a new item into the hash table.
// Returns NULL on duplicate item.
Node* hashtable_insert(HashTable* table, int key);

// Lookup an item inside the hash table.
// Returns the Node representing the item, else NULL.
Node* hashtable_lookup(HashTable* table, int key);

// Delete an item inside the hash table.
// Returns 0 on success, else -1.
int hashtable_delete(HashTable* table, int key);

// For debugging.
void hashtable_print(HashTable* table);

#endif  // HASHTABLE_H_
