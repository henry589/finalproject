#ifndef VNODE_H
#define VNODE_H

#include <mutex>      // for std::mutex, std::lock_guard
#include <vector>     // for std::vector
#include <stdexcept>
#include <cmath>
#include "memoryPool.h"
#include "othelloNet.h"

enum Side : bool {
	BLACK = false,
	WHITE = true
};


enum Won : int {
	WHITE_PLAYER,
	BLACK_PLAYER,
	PLAYER_DRAW
};
//using side = bool;

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

	// Added mutex for per-node thread safety.
	mutable std::recursive_mutex node_mutex;

	static std::stringstream ss;
	static MemoryPool pool;
	uint64_t boardB;
	uint64_t boardW;
	int action_taken;
	uint64_t sim_visits = 0;
	Side turn;
	double sim_reward = 0.0;
	double prior = 0.0;

	static constexpr double explorationConstant = 1.414;

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
	double calc_puct(double c_puct);
	// Tree manipulation functions
	void append_child(uint64_t boardB, uint64_t boardW, const Side& turn, uint8_t action);
	vnode* get_children();
	vnode* get_next_sibling();
	vnode* get_parent();
};

#endif // VNODE_H
