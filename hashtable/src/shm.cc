#include "shm.h"

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

void* shm_init(void) {
    size_t size = sizeof(SharedMem);

    int protection = PROT_READ | PROT_WRITE;

    // A third-party process (i.e., client) has to be able to access the area.
    int visibility = MAP_SHARED | MAP_ANONYMOUS;

    int shm_fd = shm_open(SHM_ID, O_CREAT | O_RDWR, 0666);
    ftruncate(shm_fd, size);

    return mmap(NULL, size, protection, visibility, shm_fd, 0);
}

void shm_free(SharedMem* area) {
    munmap(area, sizeof(SharedMem));
    shm_unlink(SHM_ID);
}
