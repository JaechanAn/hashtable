# Hash Table Server
Currently supports the following versions.


:green_circle: : supported, :x: : not supported, :keyboard: : implementing

| Concurrency Policy | Status |
|------|--------|
| Per bucket locking | :green_circle: |
| Group bucket locking | :x: |
| Hand-over-hand locking | :green_circle: |
| Optimistic locking | :green_circle: |
| Lock-free | :keyboard: |

To build for a different concurrency policy, change the `add_compile_definitions({policy})` command in `CMakeLists.txt`
```cmake
#add_compile_definitions(BUCKET_LOCKING) # enable implementation of a naive bucket lock
add_compile_definitions(OPTIMISTIC_LOCKING) # enable implementation of an optimistic lock
```

## How to Build & Run

```sh
# Requires cmake
# 1. Clone the repository
git clone https://github.com/JaechanAn/hashtable.git

# 2. Create a build directory
mkdir build
cd build

# 3. Build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . -j

# 4. Run
cd bin

# 4-1. Ordinary server/client program execution
./server <hashtable_size> # must execute server before client
./client <num_threads> <num_ops_per_thread>

# 4-2. Tests on basic function correctness
./hashtable_test

# 4-3. Run a simple performance benchmark
./benchmark <hashtable_size> <num_ops_per_thread>
```

## Required Spec

**Server**
- Initialize a hash table of given size using command line (e.g., `./server 10`).
- Support insertion of items in the hash table.
- Hash table collisions are resolved by maintaining a linked list for each bucket/entry in the hash table.
- Supports concurrent operations (multithreading) to perform `insert`, `delete`, `lookup` operations on the hash table.
- Use readers-writer lock to ensure safety of concurrent operations, try to optimize the granularity.
- Communicates with the client program using shared memory buffer (POSIX shm).

**Client**
- Enqueue operations (`insert`, `delete`, `lookup`) to the server and operate on the hash table via shared memory buffer (POSIX shm).

## Overall Design

The server is responsible for initializing the shared memory area. Once the shared memory area is initialized and attached, the server waits for the client to join and produce workloads. The server and client communicate through the shared memory area, where a bounded-size concurrent queue handles the producer/consumer mechanism. Once the client finishes sending all the jobs, the client terminates. The server takes care of the remaining jobs and terminates. The hash table resides on the server's memory.

<img width="710" alt="스크린샷 2024-01-18 오후 2 22 43" src="https://github.com/JaechanAn/hashtable/assets/13327840/6e666b66-c35f-4030-bf6a-d4bd3d380e88">

## Hash Table

### Constraints
For the simplicity of this toy project, we define some constraints for the hash table.
- A `key` represents an item within the hash table, where the value may only be a positive integer.
- Since the number of items within the entire hash table is not specified, allocate dynamically.
- Use the modulo operation for the hash function to determine the bucket.
- Everything is in-memory and does not support a disk-based hash table.
- Dynamic shared memory is unsupported, only allow static sizing.
- Shared memory is used only for IPC.

### Representation Invariant
The linked list for handling the collisions within the hash table should support the following instruction set with correctness defined as follows.
- Forbid duplicate keys for insertion.
- If a key is inserted and never deleted, lookup should succeed.
- If a key is deleted and never inserted, lookup should fail.
- The list has to be sorted.
- The tail should be always reachable from the head.

### Design

#### Option 1 - Per Bucket Lock
Lock individual buckets.

<img width="602" alt="스크린샷 2024-01-16 오후 5 45 24" src="https://github.com/JaechanAn/hashtable_server/assets/13327840/54e60fe3-dd08-46c8-98cf-656a325ccf88">

**Properties**
- Works considerably well in cases with less contention on each bucket
- Easy to implement
- Skewed workload will increase contention on small portion of buckets
- Not scalable

#### Option 2 - Bucket Group Lock
Support a more coarse-grained locking on buckets by grouping multiple buckets to the same lock.

<img width="609" alt="스크린샷 2024-01-16 오후 5 48 42" src="https://github.com/JaechanAn/hashtable_server/assets/13327840/36b0f18b-3975-46f5-921e-b380635c53d0">

**Properties**
- Better than option 1 when number of workers are relatively small and writers don't overlap as much
- Still not scalable, vulnerable to skewed workload as well

#### Option 3 - Hand-over-hand Lock
Use hand-over-hand (i.e., chain) locking on access to each bucket's list instead of using bucket-based locking.

<img width="797" alt="스크린샷 2024-01-16 오후 5 53 21" src="https://github.com/JaechanAn/hashtable_server/assets/13327840/848ebb22-7960-4edf-a087-82490f4c7472">

**Properties**
- Doesn't hold a lock on a bucket so multiple traversals are possible within the same bucket.
- Holding consecutive two locks may trigger more contention on a skewed workload.

#### Option 4 - Optimistic Lock
Like other optimistic policies, try some operations (e.g., traversal) without locking assuming that conflicts are rare.
Then, resolve conflicts when they do occur during a validation phase.

Example of delete
1. Traverse without locks
2. Lock the consecutive nodes, predecessor, and the node to remove.
3. Check if the predecessor is still reachable from the head, making sure that the predecessor wasn't deleted before acquiring the lock.
4. Check if the node to delete is still pointed by the predecessor, making sure that nothing was inserted in between before acquiring the lock.
5. Remove the node and release locks.


#### Option 5 - Lock-free Structure

## Evaluation

### Bucket locking

<img width="752" alt="image" src="https://github.com/JaechanAn/hashtable_server/assets/13327840/5ddd7128-9a32-4ab2-8a27-fb94001609e4">

### Chain (hand-over-hand) locking

<img width="755" alt="image" src="https://github.com/JaechanAn/hashtable_server/assets/13327840/112cbd90-cf41-4c24-93af-df4d3b62c2e1">

### Optimistic locking

<img width="716" alt="image" src="https://github.com/JaechanAn/hashtable/assets/13327840/c5c9e308-ca76-4cb4-a923-6b23088ba341">
