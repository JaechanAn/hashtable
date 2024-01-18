#ifndef SHM_H_
#define SHM_H_

#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "queue.h"

#define SHM_ID "/hashtable_program_shmem"  // MUST be the same with client

typedef struct {
    OperationQueue queue;  // concurrent bounded queue
    bool is_ready;         // NOTE: only set to true after initialization of queue
} SharedMem;

void* shm_init(void);
void shm_free(SharedMem* area);

#endif /* SHM_H_ */
