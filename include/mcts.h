// vanilla monte-carlo tree search
#ifndef MCTS_H
#define MCTS_H
#include "nodeManager.h"
#include "bitBoard.h"

using namespace bitboard;

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
	void backup(vnode* lfnode, const Side& win_side, const bool& is_draw);
	Won check_winner(const Bitboard& blackBoard, const Bitboard& whiteBoard);
	bool isTerminal(vnode* node);
	void boardViewer(const Bitboard& boardB, const Bitboard& boardW);
	vnode* createValidChildren(vnode* node, int& child_count);
	vnode* createValidChild(vnode* node, int& child_count);
	bool haveValidChild(vnode* node);
};

#endif