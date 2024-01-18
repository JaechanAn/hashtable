# Hash Table Server

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
- The list doesn't have to be sorted.
- The tail should be always reachable from the head.

### Design

#### Option 1 - Naive Approach
Lock individual buckets.

<img width="602" alt="스크린샷 2024-01-16 오후 5 45 24" src="https://github.com/JaechanAn/hashtable_server/assets/13327840/54e60fe3-dd08-46c8-98cf-656a325ccf88">

**Properties**
- Works considerably well in cases with less contention on each bucket
- Easy to implement
- Skewed workload will increase contention on small portion of buckets
- Not scalable

#### Option 2 - Group Lock
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
Like other optimistic policies, try the operation without locking assuming that conflicts are rare.
Then, resolve conflicts when they do occur during a validation phase.

## Evaluation

### Naive Approach

<img width="752" alt="image" src="https://github.com/JaechanAn/hashtable_server/assets/13327840/5ddd7128-9a32-4ab2-8a27-fb94001609e4">

### Hand-over-hand locking

<img width="755" alt="image" src="https://github.com/JaechanAn/hashtable_server/assets/13327840/112cbd90-cf41-4c24-93af-df4d3b62c2e1">

