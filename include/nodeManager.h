#ifndef VNODE_H
#define VNODE_H

#include <mutex>      // for std::mutex, std::lock_guard
#include <vector>     // for std::vector
#include <stdexcept>
#include <cmath>
#include "memoryPool.h"

// Forward declaration of MemoryPool (assuming its implementation is elsewhere)
class vnode {
private:
    vnode* parent = nullptr;
    vnode* children_head = nullptr;
    vnode* sibling_next = nullptr; 
    vnode* bfs_next = nullptr; // for breadth-first-search queue use only

    // remove the specific node in the tree
    static void remove_node(vnode* node);

public:
    static std::stringstream ss;
    static MemoryPool pool;
    uint64_t boardB;
    uint64_t boardW;
    u_int64_t sim_visits = 0;
    double sim_reward = 0.0;
    double explorationConstant = 0.0;

    enum class OpType {
        TRAVERSE,
        PRUNE,
        UKNOWN
    };

    vnode() { }
    virtual ~vnode() { }

    // Custom memory management
    void* operator new(size_t size);
    void operator delete(void* p);

    // Static utility functions
    static std::string get_dot_formatted();
    static void traverse(vnode* node);
    static void BFS(vnode* node, OpType method = OpType::TRAVERSE, bool include_current_node = false);
    double calc_uct();
    // Tree manipulation functions
    void append_child(uint64_t boardB, uint64_t boardW);
    vnode* get_children();
    vnode* get_next_sibling();
    vnode* get_parent();
};

#endif // VNODE_H
