#include "../include/mcts.h"
#include "../include/misc.h"
#include <iostream>
#include "../include/bitBoard.h"

using namespace bitboard;
//select the nodes to form a path down the tree
vnode* mcts::selection(vnode* const root)
{
	vnode* curr_node = root;

	while (curr_node)
	{
		// Check if leaf (no children)
		vnode* child = curr_node->get_children();
		if (!child)
		{
			std::cout << "\nselected leaf: " << curr_node->boardB << '\n';
			return curr_node;
		}

		// Track best visited child & UCT value
		vnode* best_child = nullptr;
		double best_uct_value = -std::numeric_limits<double>::infinity();

		// Collect unvisited children here
		std::vector<vnode*> unvisited_nodes;
		unvisited_nodes.reserve(8); // Optional small reserve

		// Traverse all siblings
		for (; child; child = child->get_next_sibling())
		{
			// If unvisited, add to the vector
			if (child->sim_visits == 0)
			{
				unvisited_nodes.push_back(child);
			}
			else
			{
				// Visited: compare UCT
				double cur_uct_value = child->calc_uct();

				if (cur_uct_value > best_uct_value)
				{
					best_uct_value = cur_uct_value;
					best_child = child;
				}
				// Tie-breaker
				else if (cur_uct_value == best_uct_value && (fast_rand() % 2))
				{
					best_child = child;
				}
			}
		}

		// If unvisited children exist, pick one at random & return
		if (!unvisited_nodes.empty())
		{
			std::size_t idx = fast_rand() % unvisited_nodes.size();
			vnode* selected = unvisited_nodes[idx];

			std::cout << "\nselected leaf: " << selected->boardB
				<< ", size: " << unvisited_nodes.size() << '\n';

			return selected;
		}

		// Otherwise, go down to the best visited child
		curr_node = best_child;
	}

	// If we somehow exit the loop with no node, return nullptr
	return nullptr;
}

vnode* mcts::expansion(vnode* lfnode)
{
	
	//create valid children from current lfnode
	int valid_child_count = 0;
	vnode* children = createValidChildren(lfnode, valid_child_count);
	vnode* tmp_node = children;
	const int & nonce = getRandomNumber(0, valid_child_count - 1);
	int count = 0;

	// if there's some valid children
	while (tmp_node != nullptr)
	{
		// return the random child
		if (nonce == count)
		{
			return tmp_node;
		}
		tmp_node = tmp_node->get_next_sibling();
		++count;
	}
	
	return nullptr;
}


//void mcts::expansion(vnode* leaf) {
//	// Do NOT expand a terminal node
//	if (leaf->is_terminal()) return;
//
//	// Check if already expanded
//	if (leaf->is_fully_expanded()) return;
//
//	// Get all legal moves
//	std::vector<Move> legal_moves = leaf->get_legal_moves();
//
//	// Reserve space (optimization)
//	leaf->children.reserve(legal_moves.size());
//
//	for (const Move& move : legal_moves) {
//		vnode* new_node = new vnode(leaf, move);
//		leaf->add_child(new_node);
//	}
//
//	// Mark node as fully expanded
//	leaf->set_fully_expanded(true);
//}

void mcts::simulation()
{
}

void mcts::backup()
{
}

bool mcts::isTerminal(vnode* node)
{
	return false;
}

uint64_t mcts::placeMove(Bitboard& board, int bit_pos)
{
	return board | (1ULL << bit_pos);
}

void mcts::boardViewer(const bitboard::Bitboard& boardB, const bitboard::Bitboard& boardW)
{
	//decode the bitboard
	uint64_t boardBtmp = boardB;
	uint64_t boardWtmp = boardW;
	int n = 0;
	char board_char_arr[64] = {};
	for (int m = 0; m < 64; ++m)
	{
		board_char_arr[n++] = boardBtmp & 1 ? 'x' : boardWtmp & 1 ? 'o' : '-';
		boardBtmp >>= 1;
		boardWtmp >>= 1;
	}
	std::cout << "\npretty board:\n";

	for (int m = 7; m >= 0; --m)
	{
		for (int i = (8 * m) + 7; i >= 8 * m; --i)
		{
			std::cout << board_char_arr[i];
			if (i % 8 == 0)
				std::cout << std::endl;
		}
	}
	std::cout << "\nend board\n";
}

vnode* mcts::createValidChildren(vnode* node, int& child_count)
{
	// take current node and then check for all possible nodes
	// this will follow the othello rule
	const Bitboard& cur_boardB = node->boardB;
	const Bitboard& cur_boardW = node->boardW;
	const Bitboard& cur_side = node->turn == BLACK ? cur_boardB : cur_boardW;
	const Bitboard& alt_side = node->turn == WHITE ? cur_boardB : cur_boardW;
	const Bitboard& overlapped_board = cur_boardB | cur_boardW;
	// assume the square sq is empty
	// we have to know whether it is empty square first
	for (Square sq = SQ_A1; sq <= SQ_H8; ++sq)
	{
		// check if not empty empty
		if (overlapped_board & (1ULL << sq))
		{
			continue;
		}
		else { // empty
			// actual flips are the flips that would be done by the cur_boardB only
			const Bitboard& future_flips = actual_flips(sq, cur_side, alt_side);

			if (future_flips ^ 0) // future_flips contain something also means not equal to zero
			{
				const Bitboard& mod_cur_boardB = node->turn == BLACK ? future_flips | cur_boardB | (1ULL << sq) : ~future_flips & cur_boardB;
				const Bitboard& mod_cur_boardW = node->turn == WHITE ? future_flips | cur_boardW | (1ULL << sq) : ~future_flips & cur_boardW;
				node->append_child(mod_cur_boardB, mod_cur_boardW, !node->turn, sq);
				++child_count;
			}
		}
	}

	// Bitboard
	return node->get_children();
}