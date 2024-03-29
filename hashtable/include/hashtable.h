#ifndef HASHTABLE_H_
#define HASHTABLE_H_

#include <pthread.h>
#include <stddef.h>

typedef struct Node {
    int key;            // currently supports integer key only
    struct Node* next;  // next pointer for handling linked list style chaining
#if defined(CHAIN_LOCKING) || defined(OPTIMISTIC_LOCKING)
    pthread_rwlock_t* lock;
#endif
} Node;

Node* init_node(void);

typedef struct HashTable {
    Node** buckets;  // represents the table buckets
    int size;        // represents the bucket size, not the number of items
#ifdef BUCKET_LOCKING
    pthread_rwlock_t* bucket_locks;
#endif
} HashTable;

/*
 * Hash table control functions
 */

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

int hashtable_size(HashTable* table);

#ifdef OPTIMISTIC_LOCKING
bool validate(Node* bucket, Node* prev, Node* curr);
#endif

#endif  // HASHTABLE_H_
