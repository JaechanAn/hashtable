#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "server.h"

#define HASH_SIZE 100
#define SHM_NAME "/my_shared_memory"

typedef struct Node {
    int data;
    struct Node* next;
} Node;

typedef struct {
    Node* hash_table[HASH_SIZE];
    pthread_rwlock_t lock;
} SharedData;

void init_shared_data(SharedData* shared_data) {
    memset(shared_data->hash_table, 0, sizeof(Node*) * HASH_SIZE);
    pthread_rwlock_init(&shared_data->lock, NULL);
}

void insert_data(SharedData* shared_data, int key, int value) {
    int index = key % HASH_SIZE;

    Node* new_node = (Node*)malloc(sizeof(Node));
    new_node->data = value;
    new_node->next = NULL;

    pthread_rwlock_wrlock(&shared_data->lock);

    if (shared_data->hash_table[index] == NULL) {
        shared_data->hash_table[index] = new_node;
    } else {
        new_node->next = shared_data->hash_table[index];
        shared_data->hash_table[index] = new_node;
    }

    pthread_rwlock_unlock(&shared_data->lock);
}

void* server_thread(void* arg) {
    SharedData* shared_data = (SharedData*)arg;

    // Simulate server waiting for client requests
    while (1) {
        // Your server logic here

        // Sleep for 1 second as an example
        sleep(1);
    }

    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <hash_table_size>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int hash_table_size = atoi(argv[1]);

    // Create and initialize shared memory
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    ftruncate(shm_fd, sizeof(SharedData));
    SharedData* shared_data =
        (SharedData*)mmap(NULL, sizeof(SharedData), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    init_shared_data(shared_data);

    // Create server thread
    pthread_t server_tid;
    pthread_create(&server_tid, NULL, server_thread, (void*)shared_data);

    // Wait for client process to complete
    printf("Server is waiting for the client...\n");
    wait(NULL);

    // Clean up
    pthread_join(server_tid, NULL);
    pthread_rwlock_destroy(&shared_data->lock);
    munmap(shared_data, sizeof(SharedData));
    shm_unlink(SHM_NAME);

    return 0;
}
