#include <stdio.h>
#include <stdlib.h>

#include "shm.h"

#define NUM_PRODUCER                32
#define NUM_CONSUMER                NUM_PRODUCER
#define NUM_THREADS                 (NUM_PRODUCER + NUM_CONSUMER)
#define NUM_ENQUEUE_PER_PRODUCER    1000000
#define NUM_DEQUEUE_PER_CONSUMER    NUM_ENQUEUE_PER_PRODUCER

int main(int argc, char** argv) {

    return EXIT_SUCCESS;
}
