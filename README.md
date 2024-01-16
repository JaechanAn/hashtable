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

### Representation Invariant
The hash table should support the following instruction set with correctness defined as follows.

#### Insert

#### Delete

#### Lookup

### Design

#### Option 1
Lock individual buckets.

#### Option 2
Support a more coarse-grained locking on buckets by grouping multiple buckets to the same lock.

#### Option 3
Use hand-over-hand (i.e., chain) locking on access to each bucket's list instead of using bucket based locking.
