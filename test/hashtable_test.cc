#include "hashtable.h"

#include <gtest/gtest.h>
#include <unistd.h>

#include <string>

/*******************************************************************************
 * NOTE from TA: Jaechan An
 * The test structures stated here were written to give you and idea of what a
 * test should contain and look like. Feel free to change the code and add new
 * tests of your own. The more concrete your tests are, the easier it'd be to
 * detect bugs in the future projects.
 ******************************************************************************/

/*
 * Test basic initialization.
 * 1. Open a file and check the descriptor
 * 2. Check if the file's initial size is 10 MiB
 */
TEST(HashTableInitTest, HandlesInitialization) {
    int hashtable_size = 10;

    HashTable* table = hashtable_create(hashtable_size);

    ASSERT_TRUE(table != NULL);

    ASSERT_EQ(table->size, hashtable_size);
    for (int i = 0; i < table->size; ++i) {
        ASSERT_TRUE(table->buckets[i] != NULL);
        ASSERT_TRUE(table->buckets[i]->key == -1);
        ASSERT_TRUE(table->buckets[i]->next == NULL);
    }

    int freed = hashtable_free(table);
    ASSERT_EQ(freed, 0);
}

/*
 * TestFixture for hash table basic operation tests
 */
class HashTableBasicTest : public ::testing::Test {
protected:
    /*
     * NOTE: You can also use constructor/destructor instead of SetUp() and
     * TearDown(). The official document says that the former is actually
     * perferred due to some reasons. Checkout the document for the difference
     */
    HashTableBasicTest() {
        int hashtable_size = 100;

        table = hashtable_create(hashtable_size);
    }

    ~HashTableBasicTest() { int freed = hashtable_free(table); }

    HashTable* table;  // Hash table
};

/*
 * Test insertion
 * 1. Insert test for success
 * 2. Insert test for failure (i.e., duplicate key)
 */
TEST_F(HashTableBasicTest, Insert) {
    // Test should succeed
    for (int i = 0; i < 1000; ++i) {
        Node* inserted_node = hashtable_insert(table, i);
        ASSERT_TRUE(inserted_node != NULL);
    }

    for (int i = 0; i < 1000; ++i) {
        Node* found = hashtable_lookup(table, i);
        ASSERT_TRUE(found != NULL);
    }

    // Test should fail
    Node* should_fail = hashtable_insert(table, 1);
    ASSERT_TRUE(should_fail == NULL);
}

/*
 * Test deletion
 * 1. Delete test for success
 * 2. Delete test for failure (i.e., key not found)
 */
TEST_F(HashTableBasicTest, Delete) {
    // Test should succeed
    for (int i = 0; i < 1000; ++i) {
        Node* inserted_node = hashtable_insert(table, i);
        ASSERT_TRUE(inserted_node != NULL);
    }

    for (int i = 0; i < 1000; ++i) {
        int deleted = hashtable_delete(table, i);
        ASSERT_EQ(deleted, 0);
    }

    // Test should fail
    int should_fail = hashtable_delete(table, 1);
    ASSERT_EQ(should_fail, -1);
}

/*
 * TestFixture for hash table concurrency test
 */
class HashTableConcurrencyTest : public ::testing::Test {
protected:
    /*
     * NOTE: You can also use constructor/destructor instead of SetUp() and
     * TearDown(). The official document says that the former is actually
     * perferred due to some reasons. Checkout the document for the difference
     */
    HashTableConcurrencyTest() {
        int hashtable_size = 100;

        table = hashtable_create(hashtable_size);
        ncores = sysconf(_SC_NPROCESSORS_ONLN);
    }

    ~HashTableConcurrencyTest() { int freed = hashtable_free(table); }

    HashTable* table;  // Hash table
    int ncores;        // Number of cores
};

typedef struct ThreadArgs {
    int id;
    HashTable* table;
} ThreadArgs;

void* insert_one_func(void* thd_args) {
    ThreadArgs* args = (ThreadArgs*)thd_args;
    int id = args->id;
    HashTable* table = args->table;

    Node* node = hashtable_insert(table, 1);

    pthread_exit(NULL);
}

void* delete_func(void* id) {
    long thd_id = (long)id;

    for (int i = 0; i < 10000; ++i) {
    }

    pthread_exit(NULL);
}

void* lookup_func(void* id) {
    long thd_id = (long)id;

    for (int i = 0; i < 10000; ++i) {
    }

    pthread_exit(NULL);
}

/*
 * Test concurrent insertion
 * 1. Insert key '1' with {number of cores * 2} threads.
 * 2. Wait until all operation is done.
 * 3. Check if only one item is inserted.
 */
TEST_F(HashTableConcurrencyTest, Insert) {
    int num_threads = ncores * 2;

    pthread_t threads[num_threads];

    for (int i = 0; i < num_threads; i++) {
        ThreadArgs args;
        args.id = i;
        args.table = table;
        pthread_create(&threads[i], NULL, insert_one_func, (void**)&args);
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
}
