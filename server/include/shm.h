#include <stddef.h>

#define SHM_SEGMENT (4096)
#define SHM_ID "/hashtable_program_shmem"  // MUST be the same with client

void* shm_create(size_t size);
void shm_free(void* mem, size_t size);
