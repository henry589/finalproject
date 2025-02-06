#include <algorithm>  // for std::max
#include <cstdlib>    // for std::malloc, std::free
#include <mutex>      // for std::mutex, std::lock_guard
#include <vector>     // for std::vector
#include <stdexcept>  // for std::bad_alloc
#include <cassert>
#include <iostream>
#include <thread>
#include <cstring> // For memset and memcpy
 
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
    MemoryPool(size_t blockSize, size_t chunkSize)
        : block_size_(std::max(blockSize, sizeof(Block))),
          available_size_(chunkSize),
          freelist_(nullptr)
    {
        if (chunkSize == 0) {
            throw std::invalid_argument("Chunk size must be > 0.");
        }
    }

    // Destructor
    ~MemoryPool() {
        for (void* chunk : chunks_) {
            std::free(chunk);
        }
    }

    // Allocate a block of memory
    void* allocate() {
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
    void deallocate(void* ptr) {
        // Lock the pool before modifying freelist_
        std::lock_guard<std::mutex> lock(mutex_);

        // Insert this block back at the head of the freelist
        Block* block = static_cast<Block*>(ptr);
        block->next = freelist_;
        freelist_ = block;
    }

private:
    // Allocate an entire chunk of block_size_ * chunk_size_
    void allocateChunk() {
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
};

class vnode{



public:
    vnode *parent=nullptr;
    vnode *children_head=nullptr;
    vnode *sibling_next=nullptr; 
    static MemoryPool pool;
    u_int64_t boardB;
    u_int64_t boardW;
    void * operator new(size_t size)
    {
        return pool.allocate();
    }

    void operator delete(void *p)
    {
        return pool.deallocate(p);
    }
};


MemoryPool vnode::pool = MemoryPool(sizeof(vnode),100000);
void test()
{
    vnode * list  = NULL;
    
    // for(int m = 0; m < 1000000; ++m)
    // {
    //     vnode *hello = new vnode();
    //     hello->board = m;
    //     hello->left = list;
    //     list = hello;
    // }

    // vnode * tmplist = list;

    // while(tmplist != NULL)
    // {
    //     std::cout<<tmplist->board<<std::endl;
    //     tmplist = tmplist->left;
    // }

    vnode * parent =new vnode();
    vnode * new_head = new vnode();


    for(int i =0; i <435; ++i)
    {
        new_head = new vnode();
        new_head->boardB = i;
        new_head->sibling_next = parent->children_head;
        parent->children_head = new_head; //add sibling

    }

    vnode * asd = parent->children_head;
    while(asd != nullptr)
    {
        std::cout<< asd->boardB<<std::endl;
        asd = asd->sibling_next;
    }
    
    std::cout<<"size of vnode:"<< sizeof(vnode);

}
//node selection
void selection()
{
    // node = choose_child(current node);
    //parent to child node selection the selection method is defined later
    

}

//expansion
void expansion()
{
    // if node not termination node, expand the node

}

//simulation
void simulation()
{
    //simulation starting from this node
}

//backup
void backup()
{
    //simulation starting from this node
}


void singleThreadTest() {
    std::cout << "Running single-threaded test...\n";

    // Create a memory pool with block size 32 bytes and chunk size 10 blocks
    MemoryPool pool(32, 10);

    // Allocate some blocks
    void* block1 = pool.allocate();
    void* block2 = pool.allocate();
    void* block3 = pool.allocate();

    // Deallocate a block and allocate again (to check reuse)
    pool.deallocate(block2);
    void* block4 = pool.allocate();

    // Ensure the same block was reused
    assert(block2 == block4);

    std::cout << "Single-threaded test passed.\n";
}

void multiThreadTest() {
    std::cout << "Running multi-threaded test...\n";

    MemoryPool pool(64, 20); // Create a memory pool with block size 64 bytes and chunk size 20 blocks

    const int numThreads = 4;
    const int numAllocationsPerThread = 100;

    auto allocateAndDeallocate = [&pool]() {
        std::vector<void*> blocks;

        // Allocate multiple blocks
        for (int i = 0; i < numAllocationsPerThread; ++i) {
            blocks.push_back(pool.allocate());
        }

        // Deallocate them
        for (void* block : blocks) {
            pool.deallocate(block);
        }
    };

    // Create and run multiple threads
    std::vector<std::thread> threads;
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back(allocateAndDeallocate);
    }

    for (auto& t : threads) {
        t.join(); // Wait for all threads to finish
    }

    std::cout << "Multi-threaded test passed.\n";
}

void stressTest() {
    std::cout << "Running stress test...\n";

    MemoryPool pool(128, 50); // Create a memory pool with block size 128 bytes and chunk size 50 blocks

    const int numThreads = 8;
    const int numAllocationsPerThread = 1000;

    auto allocateAndDeallocate = [&pool]() {
        for (int i = 0; i < numAllocationsPerThread; ++i) {
            void* block = pool.allocate();
            pool.deallocate(block); // Allocate and immediately deallocate
        }
    };

    // Create and run multiple threads
    std::vector<std::thread> threads;
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back(allocateAndDeallocate);
    }

    for (auto& t : threads) {
        t.join(); // Wait for all threads to finish
    }

    std::cout << "Stress test passed.\n";
}

void valueVerificationTest() {
    std::cout << "Running value verification test...\n";

    const size_t blockSize = 32;
    const size_t chunkSize = 10;

    MemoryPool pool(blockSize, chunkSize);

    // Allocate blocks and write unique data to each
    void* blocks[chunkSize];
    for (size_t i = 0; i < chunkSize; ++i) {
        blocks[i] = pool.allocate();

        // Write data to the block
        int value = static_cast<int>(i); // Example unique value
        std::memset(blocks[i], value, blockSize); // Fill block with the value
    }

    // Verify that data is preserved
    for (size_t i = 0; i < chunkSize; ++i) {
        int value = static_cast<int>(i); // Expected value
        char expected[blockSize];
        std::memset(expected, value, blockSize); // Create expected pattern

        // Compare memory content
        assert(std::memcmp(blocks[i], expected, blockSize) == 0);
    }

    // Deallocate all blocks
    for (size_t i = 0; i < chunkSize; ++i) {
        pool.deallocate(blocks[i]);
    }

    std::cout << "Value verification test passed.\n";
}

int main() {
    // try {
    //     valueVerificationTest();

    //     std::cout << "All tests passed successfully!\n";
    // } catch (const std::exception& e) {
    //     std::cerr << "Test failed: " << e.what() << '\n';
    //     return 1;
    // }
    test();
    return 0;
}