
#include "../include/memoryPool.h"
 

    // Constructor
    MemoryPool::MemoryPool(size_t blockSize, size_t chunkSize)
        : block_size_(std::max(blockSize, sizeof(Block))),
          available_size_(chunkSize),
          freelist_(nullptr)
    {
        if (chunkSize == 0) {
            throw std::invalid_argument("Chunk size must be > 0.");
        }
    }

    // Destructor
    MemoryPool::~MemoryPool() {
        for (void* chunk : chunks_) {
            std::free(chunk);
        }
    }

    // Allocate a block of memory
    void* MemoryPool::allocate() {
        // Lock the pool before accessing freelist_
        std::lock_guard<std::mutex> lock(mutex_);

        // If no free blocks, allocate a new chunk
        if (!freelist_) {
            allocateChunk();
        }

        // Take the first block from the freelist
        Block* block = freelist_;
        freelist_ = freelist_->next;
        return block;
    }

    // Return a block to the pool
    void MemoryPool::deallocate(void* ptr) {
        // Lock the pool before modifying freelist_
        std::lock_guard<std::mutex> lock(mutex_);

        // Insert this block back at the head of the freelist
        Block* block = static_cast<Block*>(ptr);
        block->next = freelist_;
        freelist_ = block;
    }

    // Allocate an entire chunk of block_size_ * chunk_size_
    void MemoryPool::allocateChunk() {
        // Request a big chunk from the system
        void* chunk = std::malloc(block_size_ * available_size_);
        if (!chunk) {
            // Retry with smaller chunk
            size_t tmp_size = available_size_ - 1;
            while(tmp_size >= 1)
            {
                chunk = std::malloc(block_size_ * tmp_size);
                if(chunk)
                {
                    available_size_ = tmp_size;
                    break;
                }
                tmp_size -= 1;
            }
            
            //really cannot allocate any
            if(!chunk)
            {
                throw std::bad_alloc();
            }
        }
        chunks_.push_back(chunk);

        // Break the chunk into blocks, linking them into freelist_
        char* start = static_cast<char*>(chunk);
        for (size_t i = 0; i < available_size_; ++i) {
            Block* block = reinterpret_cast<Block*>(start + i * block_size_);
            block->next = freelist_;
            freelist_ = block;
        }
    }




