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

// expand the current node and return the randomly selected node for simulation
vnode* mcts::expansion(vnode* lfnode, const exp_mode& exp_mode)
{
	if (exp_mode == EXPANSION_FULL)
	{
		//create valid children from current lfnode
		int valid_child_count = 0;
		vnode* tmp_node = createValidChildren(lfnode, valid_child_count);
		int nonce = valid_child_count > 0 ? getRandomNumber(0, valid_child_count - 1) : -1;
		int count = 0;

		// if we unable to expand, might not be terminal yet so we retry by switching the turn, this
		// is a hard re-set of the turn
		if (tmp_node == nullptr)
		{
			lfnode->turn = !lfnode->turn;
			vnode* switch_player_node = createValidChildren(lfnode, valid_child_count);
			nonce = valid_child_count > 0 ? getRandomNumber(0, valid_child_count - 1) : -1;
			// if switched player also fail means this is most probably a terminal state
			if (switch_player_node == nullptr)
			{
				return nullptr;
			}
			else {
				// revive the expansion process
				tmp_node = switch_player_node;
			}
		}

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
	}
	else if (exp_mode == EXPANSION_SINGLE)
	{
		int valid_child_count = 0;
		vnode* child = createValidChild(lfnode, valid_child_count);
		// if we unable to expand, might not be terminal yet so we retry by switching the turn, this
		// is a hard re-set of the turn
		if (child == nullptr)
		{
			lfnode->turn = !lfnode->turn;
			// return the switched player child
			return createValidChild(lfnode, valid_child_count);
		}
		return child;
	}

	return nullptr;
}

void mcts::simulation(vnode* exp_node)
{
	// if exp_node is null means it is a terminal node, directly skip simulation because no any
	// simulations could be done here
	if (exp_node != nullptr)
	{
		//play the game here with random move
		//determine valid moves and play
		//still we also need to check every possible square
		//initial area
		Bitboard current_player = 0;
		Bitboard opponent = 0;
		Bitboard overlapped_board = 0;
		Bitboard track_boardB = exp_node->boardB;
		Bitboard track_boardW = exp_node->boardW;
		bool current_turn = exp_node->turn;
		const int terminate_Criterion = 2;
		int terminal_count = 0;
		// first assume simulation running forever
		while (true)
		{
			bool chosen_once = false;
			current_player = current_turn == BLACK ? track_boardB : track_boardW;
			opponent = current_turn == WHITE ? track_boardB : track_boardW;
			overlapped_board = track_boardB | track_boardW;
			Bitboard mod_cur_boardB = 0;
			Bitboard mod_cur_boardW = 0;
			// this is each iteration of simulation
			for (Square sq = SQ_A1; sq <= SQ_H8; ++sq)
			{
				// check if not empty empty
				if (overlapped_board & (1ULL << sq))
				{
					continue;
				}
				else { // square is empty
					Bitboard future_flips = actual_flips(sq, current_player, opponent);

					// if this square contain valid move
					if (future_flips ^ 0)
					{
						// we let it do at least once, the rest whether to update the selection
						// depends on probability
						if (chosen_once == false || random_bool(0.5))
						{
							mod_cur_boardB = current_turn == BLACK ? future_flips | track_boardB | (1ULL << sq) : ~future_flips & track_boardB;
							mod_cur_boardW = current_turn == WHITE ? future_flips | track_boardW | (1ULL << sq) : ~future_flips & track_boardW;

							chosen_once = true;
							terminal_count = 0; // reset termination count
						}
					}
				}
			}
			//make move and board flipped in track_board
			track_boardB = mod_cur_boardB;
			track_boardW = mod_cur_boardW;
			boardViewer(track_boardB, track_boardW);

			//increase the terminal count when no child was found
			if (chosen_once == false) ++terminal_count;

			// when both(x2) sides do not have any valid moves to go, means a terminal state achieved
			if (terminal_count >= 2)
				break;

			//update the current player and opponent here when moving to the next move
			current_turn = !current_turn;
		}

		// after breaking from the above termination, we try to check the current state's winner
		boardViewer(track_boardB, track_boardW);

	}
}

void mcts::backup()
{
}

bool mcts::isTerminal(vnode* node)
{
	return false;
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

// create valid full children by checking every square whether it is valid or not
vnode* mcts::createValidChildren(vnode* node, int& child_count)
{
	// take current node and then check for all possible nodes this will follow the othello rule
	Bitboard  cur_boardB = node->boardB;
	Bitboard  cur_boardW = node->boardW;
	Bitboard  cur_side = node->turn == BLACK ? cur_boardB : cur_boardW;
	Bitboard  alt_side = node->turn == WHITE ? cur_boardB : cur_boardW;
	Bitboard  overlapped_board = cur_boardB | cur_boardW;

	// assume the square sq is empty we have to know whether it is empty square first
	for (Square sq = SQ_A1; sq <= SQ_H8; ++sq)
	{
		// check if not empty empty
		if (overlapped_board & (1ULL << sq))
		{
			continue;
		}
		else { // empty
			// actual flips are the flips that would be done by the cur_boardB only
			const Bitboard future_flips = actual_flips(sq, cur_side, alt_side);

			if (future_flips ^ 0) // future_flips contain something also means not equal to zero
			{
				Bitboard mod_cur_boardB = node->turn == BLACK ? future_flips | cur_boardB | (1ULL << sq) : ~future_flips & cur_boardB;
				Bitboard mod_cur_boardW = node->turn == WHITE ? future_flips | cur_boardW | (1ULL << sq) : ~future_flips & cur_boardW;
				node->append_child(mod_cur_boardB, mod_cur_boardW, !node->turn, sq);
				++child_count;
			}
		}
	}

	// return the valid children
	return node->get_children();
}

// create valid one child only by checking square whether it is valid or not with entropy selection
vnode* mcts::createValidChild(vnode* node, int& child_count)
{
	// take current node and then check for all possible nodes this will follow the othello rule
	Bitboard  cur_boardB = node->boardB;
	Bitboard  cur_boardW = node->boardW;
	Bitboard  cur_side = node->turn == BLACK ? cur_boardB : cur_boardW;
	Bitboard  alt_side = node->turn == WHITE ? cur_boardB : cur_boardW;
	Bitboard  overlapped_board = cur_boardB | cur_boardW;

	std::vector<Square> potentialSquare;
	// assume the square sq is empty we have to know whether it is empty square first
	for (Square sq = SQ_A1; sq <= SQ_H8; ++sq)
	{
		// check if not empty empty
		if (overlapped_board & (1ULL << sq))
		{
			continue;
		}
		else { // empty
			// actual flips are the flips that would be done by the cur_boardB only
			const Bitboard future_flips = actual_flips(sq, cur_side, alt_side);

			if (future_flips ^ 0) // future_flips contain something also means not equal to zero
			{
				potentialSquare.push_back(sq);
				++child_count;
			}
		}
	}

	if (!potentialSquare.empty())
	{
		std::size_t idx = fast_rand() % potentialSquare.size();
		Square sq_selected = potentialSquare[idx];

		const Bitboard future_flips = actual_flips(sq_selected, cur_side, alt_side);
		Bitboard mod_cur_boardB = node->turn == BLACK ? future_flips | cur_boardB | (1ULL << sq_selected) : ~future_flips & cur_boardB;
		Bitboard mod_cur_boardW = node->turn == WHITE ? future_flips | cur_boardW | (1ULL << sq_selected) : ~future_flips & cur_boardW;
		node->append_child(mod_cur_boardB, mod_cur_boardW, !node->turn, sq_selected);
	}

	return node->get_children();

	// return nullptr means no valid child exist
	return nullptr;
}

bool mcts::haveValidChild(vnode* node)
{		
	// take current node and then check for all possible nodes
	// this will follow the othello rule
	Bitboard  cur_boardB = node->boardB;
	Bitboard  cur_boardW = node->boardW;
	Bitboard  cur_side = node->turn == BLACK ? cur_boardB : cur_boardW;
	Bitboard  alt_side = node->turn == WHITE ? cur_boardB : cur_boardW;
	Bitboard  overlapped_board = cur_boardB | cur_boardW;

	// assume the square sq is empty we have to know whether it is empty square first
	for (Square sq = SQ_A1; sq <= SQ_H8; ++sq)
	{
		// check if not empty empty
		if (overlapped_board & (1ULL << sq))
		{
			continue;
		}
		else { // empty
			// actual flips are the flips that would be done by the cur_boardB only
			const Bitboard future_flips = actual_flips(sq, cur_side, alt_side);

			if (future_flips ^ 0) // future_flips contain something also means not equal to zero
			{
				return true;
			}
		}
	}

	return false;
}