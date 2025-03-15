// vanilla monte-carlo tree search
#ifndef MCTS_H
#define MCTS_H
#include "nodeManager.h"
#include "bitBoard.h"

class mcts {
private:

	const double WIN = 1.0;
	const double LOSE = -1.0;
	const double DRAW = 0.5;
public:
	enum exp_mode : int {
		EXPANSION_FULL,
		EXPANSION_SINGLE
	};
	vnode* selection(vnode* root);
	vnode* expansion(vnode* lfnode, const exp_mode& exp_mode = EXPANSION_FULL);
	void simulation(vnode* exp_node);
	void backup(vnode* lfnode, const bool& term_side, const bool& is_draw);
	bool isTerminal(vnode* node);
	void boardViewer(const bitboard::Bitboard& boardB, const bitboard::Bitboard& boardW);
	vnode* createValidChildren(vnode* node, int& child_count);
	vnode* createValidChild(vnode* node, int& child_count);
	bool haveValidChild(vnode* node);
};

#endif