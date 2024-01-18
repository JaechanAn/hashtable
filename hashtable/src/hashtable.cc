#include "hashtable.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Node* init_node() {
    Node* node = (Node*)malloc(sizeof(Node));
    assert(node != NULL);

    node->key = -1;
    node->next = NULL;

#ifdef FINE_GRAINED_LOCKING
    node->lock = (pthread_rwlock_t*)malloc(sizeof(pthread_rwlock_t));
    assert(node->lock != NULL);

    pthread_rwlock_init(node->lock, NULL);
#endif /* FINE_GRAINED_LOCKING */

    return node;
}

size_t hashtable_estimate_size(size_t hashtable_size) {
    size_t size = 0;

    // size += sizeof(Node) * MAX_CHAIN_LENGTH * (hashtable_size + 1 /* sentinel head */);

    return size;
}

HashTable* hashtable_create(int size) {
    assert(size > 0);

    HashTable* table = (HashTable*)malloc(sizeof(HashTable));
    if (table == NULL) {
        return NULL;
    }

    table->buckets = (Node**)malloc(sizeof(Node*) * size);
    if (table->buckets == NULL) {
        return NULL;
    }

#ifdef COARSE_GRAINED_LOCKING
    table->bucket_locks = (pthread_rwlock_t*)malloc(sizeof(pthread_rwlock_t) * size);
    if (table->bucket_locks == NULL) {
        return NULL;
    }
#endif /* COARSE_GRAINED_LOCKING */

    table->size = size;

    for (int i = 0; i < size; ++i) {
        // For each bucket, we include an empty head node for convenience
        Node* head = init_node();
        table->buckets[i] = head;

#ifdef COARSE_GRAINED_LOCKING
        pthread_rwlock_init(&table->bucket_locks[i], NULL);
#endif /* COARSE_GRAINED_LOCKING */
    }

    return table;
}

int hashtable_free(HashTable* table) {
    assert(table != NULL);

    for (int i = 0; i < table->size; ++i) {
        Node* curr = table->buckets[i];
        Node* next;
        while (curr != NULL) {
            next = curr->next;
            free(curr);
            curr = next;
        }
    }

    return 0;
}

int hash_func(int key, int size) {
    // currently use modulo operation
    return key % size;
}

Node* hashtable_insert(HashTable* table, int key) {
    assert(table != NULL);
    assert(key >= 0);

    int index = hash_func(key, table->size);
    Node* bucket = table->buckets[index];

#ifdef COARSE_GRAINED_LOCKING
    pthread_rwlock_wrlock(&table->bucket_locks[index]);
#elif FINE_GRAINED_LOCKING
    pthread_rwlock_wrlock(bucket->lock);
#endif

    Node* curr = bucket->next;
    Node* prev = bucket;
    while (curr != NULL) {
#ifdef FINE_GRAINED_LOCKING
        pthread_rwlock_wrlock(curr->lock);
#endif
        if (curr->key == key) {
            // Found a duplicate key, just announce failure
#ifdef COARSE_GRAINED_LOCKING
            pthread_rwlock_unlock(&table->bucket_locks[index]);
#elif FINE_GRAINED_LOCKING
            pthread_rwlock_unlock(prev->lock);
            pthread_rwlock_unlock(curr->lock);
#endif
            return NULL;
        } else if (curr->key > key) {
            // Found a position to insert
            break;
        }

#ifdef FINE_GRAINED_LOCKING
        pthread_rwlock_unlock(prev->lock);
#endif
        prev = curr;
        curr = curr->next;
    }

    assert(prev != NULL);

    Node* new_node = init_node();
    new_node->key = key;
    new_node->next = curr;
    prev->next = new_node;

#ifdef COARSE_GRAINED_LOCKING
    pthread_rwlock_unlock(&table->bucket_locks[index]);
#elif FINE_GRAINED_LOCKING
    pthread_rwlock_unlock(prev->lock);
    if (curr != NULL) {
        pthread_rwlock_unlock(curr->lock);
    }
#endif

    return new_node;
}

Node* hashtable_lookup(HashTable* table, int key) {
    assert(table != NULL);
    assert(key >= 0);

    int index = hash_func(key, table->size);
    Node* bucket = table->buckets[index];

#ifdef COARSE_GRAINED_LOCKING
    pthread_rwlock_rdlock(&table->bucket_locks[index]);
#elif FINE_GRAINED_LOCKING
    pthread_rwlock_rdlock(bucket->lock);

    Node* prev = bucket;
#endif

    Node* curr = bucket->next;
    while (curr != NULL) {
#ifdef FINE_GRAINED_LOCKING
        pthread_rwlock_rdlock(curr->lock);
#endif
        if (curr->key == key) {
            // Found a match
#ifdef COARSE_GRAINED_LOCKING
            // NOTE: there's still a problem between the deletion where a node
            // may be found, but after return, the node might be deleted by a
            // concurrent operation. However, returning the node is considered
            // the linearization point for success of lookup in this project.
            pthread_rwlock_unlock(&table->bucket_locks[index]);
#elif FINE_GRAINED_LOCKING
            pthread_rwlock_unlock(prev->lock);
            pthread_rwlock_unlock(curr->lock);
#endif
            return curr;
        } else if (curr->key > key) {
            // Key Not found
            break;
        }

#ifdef FINE_GRAINED_LOCKING
        pthread_rwlock_unlock(prev->lock);
        prev = curr;
#endif
        curr = curr->next;
    }

#ifdef COARSE_GRAINED_LOCKING
    pthread_rwlock_unlock(&table->bucket_locks[index]);
#elif FINE_GRAINED_LOCKING
    pthread_rwlock_unlock(prev->lock);
    if (curr != NULL) {
        pthread_rwlock_unlock(curr->lock);
    }
#endif

    return NULL;
}

int hashtable_delete(HashTable* table, int key) {
    assert(table != NULL);
    assert(key >= 0);

    int index = hash_func(key, table->size);
    Node* bucket = table->buckets[index];

#ifdef COARSE_GRAINED_LOCKING
    pthread_rwlock_wrlock(&table->bucket_locks[index]);
#elif FINE_GRAINED_LOCKING
    pthread_rwlock_wrlock(bucket->lock);
#endif

    Node* curr = bucket->next;
    Node* prev = bucket;
    while (curr != NULL) {
#ifdef FINE_GRAINED_LOCKING
        pthread_rwlock_wrlock(curr->lock);
#endif
        if (curr->key == key) {
            break;
        } else if (curr->key > key) {
            // Key not found.
#ifdef COARSE_GRAINED_LOCKING
            pthread_rwlock_unlock(&table->bucket_locks[index]);
#elif FINE_GRAINED_LOCKING
            pthread_rwlock_unlock(prev->lock);
            pthread_rwlock_unlock(curr->lock);
#endif
            return -1;
        }
#if FINE_GRAINED_LOCKING
        pthread_rwlock_unlock(prev->lock);
#endif
        prev = curr;
        curr = curr->next;
    }

    assert(prev != NULL);

    if (curr == NULL) {
        // Could not find a matching key
#ifdef COARSE_GRAINED_LOCKING
        pthread_rwlock_unlock(&table->bucket_locks[index]);
#elif FINE_GRAINED_LOCKING
        pthread_rwlock_unlock(prev->lock);
#endif
        return -1;
    }

    // logical deletion
    prev->next = curr->next;
    curr->next = NULL;

#ifdef COARSE_GRAINED_LOCKING
    // release before physical deletion
    pthread_rwlock_unlock(&table->bucket_locks[index]);
#elif FINE_GRAINED_LOCKING
    pthread_rwlock_unlock(prev->lock);
    pthread_rwlock_unlock(curr->lock);
#endif

    // phyisical deletion
    free(curr);

    return 0;
}

void hashtable_print(HashTable* table) {
    assert(table != NULL);

    for (int i = 0; i < table->size; ++i) {
        printf("bucket[%d]->", i);

        Node* bucket = table->buckets[i];
        Node* curr = bucket->next;
        while (curr != NULL) {
            printf("[%d]->", curr->key);
            curr = curr->next;
        }
        printf("(NULL)\n");
    }
}
