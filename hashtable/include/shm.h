#ifndef SHM_H_
#define SHM_H_

#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "queue.h"

#define SHM_ID "/hashtable_program_shm"

typedef struct {
    OperationQueue queue;   // concurrent bounded queue
    bool is_ready;          // NOTE: Set to true after server queue initialization is done
    bool should_terminate;  // NOTE: Set to true after client workload is done
} SharedMem;

void* shm_init(void);
void* shm_attach(void);
void shm_free(SharedMem* area);

#endif /* SHM_H_ */
