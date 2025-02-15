#ifndef MEMPOOL_H
#define MEMPOOL_H


#include <mutex>      // for std::mutex, std::lock_guard
#include <vector>     // for std::vector

// Forward declaration of MemoryPool (assuming its implementation is elsewhere)
class MemoryPool {
private:
    // Each free block is represented as a linked list node
    struct Block {
        Block* next;
    };

    size_t block_size_;              // Size of each block
    size_t available_size_;              // Number of blocks per chunk
    std::vector<void*> chunks_;      // Pointers to all allocated chunks
    Block* freelist_;                // Head of free-list
    std::mutex mutex_;               // For thread safety

public:
    // Constructor
    MemoryPool(size_t blockSize, size_t chunkSize);

    // Destructor
    ~MemoryPool();

    // Allocate a block of memory
    void* allocate();

    // Return a block to the pool
    void deallocate(void* ptr) ;

private:
    // Allocate an entire chunk of block_size_ * chunk_size_
    void allocateChunk() ;
};

#endif // MEMPOOL_H
