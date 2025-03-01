// vanilla monte-carlo tree search
#ifndef MCTS_H
#define MCTS_H
#include "nodeManager.h"
#include "bitBoard.h"

class mcts {
public:
	vnode* currentTree;
	vnode* selection(vnode* root);
	vnode* expansion(vnode* lfnode);
	void simulation();
	void backup();
	bool isTerminal(vnode* node);
	vnode* createValidChildren(vnode* node, int& child_count);
	void boardViewer(const bitboard::Bitboard& boardB, const bitboard::Bitboard& boardW);
	uint64_t placeMove(uint64_t& board, int  bit_pos);
};

#endif