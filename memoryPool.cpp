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
private:
    vnode *link_next = nullptr; // for deletion queue use only
   // remove the last child in the children_head list, First-in-first-out style
    void remove_node()
    {
        if(children_head != nullptr)
        {
            delete(children_head);

            children_head = children_head->sibling_next;
        }
    }

    // remove the specific node in the tree
    static void remove_node(vnode * node)
    {
        if(node != nullptr && node->parent != nullptr)
        {
            vnode * ptr = node->parent->children_head;
            vnode * prev_node = nullptr;
            while(ptr != nullptr )
            {
                                    // std::cout<<"info:"<<ptr->boardB<<std::endl;

                if(ptr == node)
                {
                    // removal of first child
                    if(prev_node == nullptr)
                        node->parent->children_head = ptr->sibling_next;
                    else
                        prev_node -> sibling_next = ptr -> sibling_next;
                    delete(node);
                    std::cout<<"successfully remove child:"<<node->boardB<<",parent is:"<< node->parent->boardB<<std::endl;
                    break;
                }
                prev_node = ptr;
                ptr = ptr->sibling_next;
            }
        }
        else if(node->parent == nullptr) //when parent is null means it is a root node
        {
            //then just delete the node will do
                        std::cout<<"root point successfully removed:"<<node->boardB<<std::endl;

            delete(node);

        }
    }

public:
    vnode *parent=nullptr;
    vnode *children_head=nullptr;
    vnode *sibling_next=nullptr; 
    static MemoryPool pool;
    u_int64_t boardB;
    u_int64_t boardW;
    vnode (){
    }

    void * operator new(size_t size)
    {
        return pool.allocate();
    }

    void operator delete(void *p)
    {
        return pool.deallocate(p);
    }

    static void prune(vnode * node, bool include_current_node = false)
    {
        vnode * deletion_queue = nullptr;
        vnode * enqueue_ptr = nullptr;
        vnode * dequeue_ptr = nullptr;
        vnode * tmp_node = node;
        // ok pruning section
        while(tmp_node != nullptr)
        {
            // enqueue the children
            vnode * children = tmp_node->get_children();
            while(children != nullptr)
            {
                // first element in queue
                if(deletion_queue == nullptr)
                {
                    deletion_queue = children;
                    deletion_queue->link_next = nullptr;
                    enqueue_ptr = deletion_queue;
                }
                else
                {
                    enqueue_ptr->link_next = children;
                    enqueue_ptr = children;
                    enqueue_ptr->link_next = nullptr;
                }
                // std::cout<<"chd:"<<children->boardB<<"p,"<<children->parent->boardB<<",c_head:"<<children_head->boardB<<std::endl;

                children = children->sibling_next;
            }

            vnode * ptr = deletion_queue;
        
            //deque to start delete child
            dequeue_ptr = deletion_queue;

            // move the pointer to the next
            if(deletion_queue != nullptr)
            {
                deletion_queue = deletion_queue->link_next;
                remove_node(dequeue_ptr);        
            }

            tmp_node = dequeue_ptr;
        }
        //done pruning section,
        //now check whether need to do current node pruning
        if(include_current_node)
        {
            remove_node(node);
        }
    }
    // append child to the children head, the link name is 'sibling_next', append both black and white bitboards
    void append_child(u_int64_t boardB, u_int64_t boardW )
    {
        vnode *child_node = new vnode();
        child_node->boardB = boardB;
        child_node->boardW = boardW;
        child_node->sibling_next = children_head;
        child_node->parent = this;
        children_head = child_node; 
    }

 
    // always point to the head of the children list
    vnode * get_children()
    {
        return children_head;
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

    vnode * parent_node = new vnode();
    parent_node->boardB = 0x1231;
    std::cout<<"start:"<<std::endl;
    // this is a routine to simulate addition of sibling to the children head
    for(int i =0; i <30; ++i)
    {
        parent_node->append_child(i, i);

    }
    std::cout<<"done:"<<std::endl;

    // to simulate the rollout of the children
    vnode * asd = parent_node->get_children();
    while(asd != nullptr)
    {
        std::cout<< asd->boardB<<std::endl;
        asd = asd->sibling_next;
    }
    
    // parent_node->prune();
    //     parent_node->prune();

       vnode * asd2 = parent_node->get_children();
       std::cout<<"second:"<<std::endl;
       int n = 0;
    while(asd2!= nullptr)
    {
        std::cout<< asd2->boardB<<std::endl;
        asd2 = asd2->sibling_next;

        n++;
    }
    // vnode::prune(parent_node);
       std::cout<<"third:"<<std::endl;
asd2 = parent_node->get_children();
    vnode * selected_node = nullptr;
    vnode * nextGen = nullptr;
        vnode * nextnextGen = nullptr;
vnode * record = nullptr;
    n = 0;
        while(asd2!= nullptr)
    {

        std::cout<< asd2->boardB<<",p:"<<asd2->parent->boardB<<std::endl;
        asd2 = asd2->sibling_next;
        if(n == 3) 
        {
            selected_node =asd2;
            selected_node->append_child(0x88,0x76);
                        selected_node->append_child(0x89,0x76);

            selected_node->append_child(0x8A,0x76);
            nextGen = selected_node->get_children();

            nextGen->append_child(0x92,0x92);
            nextGen->append_child(0x93,0x93);
            
            nextnextGen = nextGen->get_children();
                        record = nextnextGen->sibling_next;
            nextnextGen->sibling_next->append_child(0x99,0x00);
            nextnextGen->append_child(0x832,0x00);
            nextnextGen->append_child(0x123,0x00);

        }
        n++;
    }

    //    std::cout<<"\n\ndelete subtree simulation:"<<"sel node:"<<nextGen->boardB<<std::endl<<std::endl;

    // vnode::prune(nextGen);
    // asd2 = record->get_children();


    // while(asd2!= nullptr)
    // {
    //     std::cout<< asd2->boardB<<",parent1:"<<asd2->parent->boardB<<std::endl;
    //     asd2 = asd2->sibling_next;
    //     n++;
    // }

       std::cout<<"\n\ndelete subtree simulation:selected node:"<<selected_node->boardB<<std::endl<<std::endl;

    vnode::prune(selected_node, true);

       asd2 = selected_node->get_children();
            //   asd2 = nextnextGen->get_children();

    while(asd2!= nullptr)
    {
        std::cout<< asd2->boardB<<",parent2:"<<asd2->parent->boardB<<std::endl;
        asd2 = asd2->sibling_next;
        n++;
    }

    // if(selected_node != nullptr)
    //         std::cout<< selected_node->boardB<<",parent_sel:"<<selected_node->parent->boardB<<std::endl;

       std::cout<<"\n\ndelete subtree simulation:parent_node node:"<<parent_node->boardB<<std::endl<<std::endl;

    vnode::prune(parent_node, true);

    //simulate a binary tree creation
    const int max_child = 2;
    for(int depth = 0; depth<4; ++depth)
    {
        // parent_node = new vnode();
    }


           asd2 = parent_node->get_children();
            //   asd2 = nextnextGen->get_children();

    while(asd2!= nullptr)
    {
        std::cout<< asd2->boardB<<",parent3:"<<asd2->parent->boardB<<std::endl<<std::endl;
        asd2 = asd2->sibling_next;
        n++;
    }

    //generate dot format



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