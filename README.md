# Hash Table Server

## Required Spec
- Initialize a hash table of given size using command line (e.g., `./server 10`).
- Support insertion of items in the hash table.
- Hash table collisions are resolved by maintaining a linked list for each bucket/entry in the hash table.
- Supports concurrent operations (multithreading) to perform `insert`, `delete`, `lookup` operations on the hash table.
- Use readers-writer lock to ensure safety of concurrent operations, try to optimize the granularity.
- Communicates with the client program using shared memory buffer (POSIX shm).

## Hash Table

### Constraints
For the simplicity of this toy project, we define some constraints for the hash table.
- A `key` represents an item within the hash table, where the value may only be a positive integer.
- Since the number of items within the entire hash table is not specified, allocate dynamically.
- Use modulo operation for hash function to determine the bucket.
- Everthing is in-memory and does not support disk-based hash table.

### Representation Invariant
The hash table should support the following instruction set with correctness defined as follows.

#### Insert

#### Delete

#### Lookup

### Design

#### Option 1
Lock individual buckets.   

<img width="602" alt="스크린샷 2024-01-16 오후 5 45 24" src="https://github.com/JaechanAn/hashtable_server/assets/13327840/54e60fe3-dd08-46c8-98cf-656a325ccf88">

#### Option 2
Support a more coarse-grained locking on buckets by grouping multiple buckets to the same lock.   

<img width="609" alt="스크린샷 2024-01-16 오후 5 48 42" src="https://github.com/JaechanAn/hashtable_server/assets/13327840/36b0f18b-3975-46f5-921e-b380635c53d0">

#### Option 3
Use hand-over-hand (i.e., chain) locking on access to each bucket's list instead of using bucket based locking.   

<img width="797" alt="스크린샷 2024-01-16 오후 5 53 21" src="https://github.com/JaechanAn/hashtable_server/assets/13327840/848ebb22-7960-4edf-a087-82490f4c7472">

