#ifndef SHM_H_
#define SHM_H_

#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "queue.h"

#define SHM_ID "/hashtable_program_shm"

typedef struct {
    OperationQueue queue;    // concurrent bounded queue
    int num_threads;         // number of threads
    int num_ops_per_thread;  // number of operations per thread
    bool client_is_ready;
    bool server_is_ready;
} SharedMem;

void* shm_create(void);
void shm_init(SharedMem* area);
void* shm_attach(void);
void shm_free(SharedMem* area);

#endif /* SHM_H_ */
