#include "shm.h"

#include <assert.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

void* shm_create(void) {
    size_t size = sizeof(SharedMem);

    int protection = PROT_READ | PROT_WRITE;

    // A third-party process (i.e., client/server) has to be able to access the area.
    // Thus, do not specify MAP_ANONYMOUS flag.
    int visibility = MAP_SHARED;

    int shm_fd = shm_open(SHM_ID, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        return NULL;
    }
    ftruncate(shm_fd, size);

    SharedMem* area = (SharedMem*)mmap(NULL, size, protection, visibility, shm_fd, 0);
    assert(area != MAP_FAILED);

    return area;
}

void shm_init(SharedMem* area) { memset(area, 0, sizeof(SharedMem)); }

void* shm_attach(void) {
    size_t size = sizeof(SharedMem);
    int protection = PROT_READ | PROT_WRITE;
    int visibility = MAP_SHARED;

    int shm_fd = shm_open(SHM_ID, O_RDWR, 0666);
    if (shm_fd == -1) {
        return NULL;
    }

    SharedMem* area = (SharedMem*)mmap(NULL, size, protection, visibility, shm_fd, 0);
    assert(area != MAP_FAILED);

    return area;
}

void shm_free(SharedMem* area) {
    munmap(area, sizeof(SharedMem));
    shm_unlink(SHM_ID);
}
