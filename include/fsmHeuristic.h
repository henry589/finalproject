#include <iostream>
#include <vector>
#include <limits>
#include <cmath>
#include <cassert>

// Include  bitBoard + flipping logic headers
// Adjust these #includes to match  project paths:
#include "../include/bitBoard.h"   // for actual_flips(...) etc.
#include "../include/mcts.h"       // if need definitions (Square, etc.)
#include "../include/misc.h"       // for popcount, or random if needed

namespace FSM {
    // In  code, a "Square" is typically an integer [0..63], indexing the 8x8 board.
    static const int BOARD_SIZE = 8;

    // Simple inline helpers:
    inline int row_of(int sq) { return sq / BOARD_SIZE; }
    inline int col_of(int sq) { return sq % BOARD_SIZE; }

    // Check if a square is in a board corner:
    inline bool is_corner(int sq) {
        // corners: 0, 7, 56, 63 on a 8x8
        return (sq == 0 || sq == 7 || sq == 56 || sq == 63);
    }

    // Check if a square is on any board edge (but not corner):
    inline bool is_edge(int sq) {
        int r = row_of(sq), c = col_of(sq);
        // If r=0 or r=7 or c=0 or c=7 => it's an edge. Corners are also edges,
        // but we might treat corners separately for a bigger reward.
        // We'll call it "edge" if it's on an edge but not corner:
        if ((r == 0 || r == 7 || c == 0 || c == 7) && !is_corner(sq)) {
            return true;
        }
        return false;
    }

    // This enumerates all valid moves using  'actual_flips' function from bitBoard.cpp.
    // 'turn' => if BLACK => we treat boardB as current player, boardW as opponent, etc.
    std::vector<Square> getValidMoves(Bitboard boardB, Bitboard boardW, bool turn)
    {
        std::vector<Square> valids;
        valids.reserve(32);

        // Current player's occupancy if turn == BLACK, else White
        Bitboard cur_side = turn == BLACK ? boardW : boardB;
        Bitboard alt_side = turn == WHITE ? boardW : boardB;
        Bitboard overlapped = boardB | boardW;

        for (Square sq = SQ_A1; sq < SQ_H8; ++sq) {
            // Skip squares already occupied
            if (overlapped & (1ULL << sq)) {
                continue;
            }
            // Use flipping logic to see if this is a valid move
            Bitboard flips = actual_flips(sq, cur_side, alt_side);
            if (flips != 0ULL) {
                valids.push_back(sq);
            }
        }
        return valids;
    }

    // We define a simple state machine for demonstration:
    enum class FSMState {
        CHECK_CORNER,    // first prefer corner moves
        CHECK_EDGE,      // next prefer edges
        CHECK_MOBILITY,  // then pick the move that flips the most pieces
        FALLBACK,        // if no valid moves, pass
        DONE
    };

    /**
     * \brief  pickMove_KaneFSM:  Main function that picks a move based on an FSM approach.
     * \param  boardB, boardW:    Black & White bitboards
     * \param  turn:              true=BLACK, false=WHITE
     * \return chosen square [0..63], or -1 if pass
     */
    int pickMove_KaneFSM(Bitboard boardB, Bitboard boardW, bool turn)
    {
        std::vector<Square> valids = getValidMoves(boardB, boardW, turn);
        if (valids.empty()) {
            // no moves => pass
            return -1;
        }

        FSMState state = FSMState::CHECK_CORNER;
        int chosenMove = -1;
        int bestFlipCount = -1; // for MOBILITY state

        while (state != FSMState::DONE)
        {
            switch (state) {

            case FSMState::CHECK_CORNER:
            {
                for (int move : valids) {
                    if (is_corner(move)) {
                        //std::cout << "[KaneFSM] Found CORNER move at " << move << std::endl;
                        chosenMove = move;
                        break;
                    }
                }
                if (chosenMove != -1) {
                    state = FSMState::DONE;
                }
                else {
                    state = FSMState::CHECK_EDGE;
                }
            } break;

            case FSMState::CHECK_EDGE:
            {
                for (int move : valids) {
                    if (is_edge(move)) {
                        //std::cout << "[KaneFSM] Found EDGE move at " << move << std::endl;
                        chosenMove = move;
                        break;
                    }
                }
                if (chosenMove != -1) {
                    state = FSMState::DONE;
                }
                else {
                    state = FSMState::CHECK_MOBILITY;
                }
            } break;

            case FSMState::CHECK_MOBILITY:
            {
                // "Mobility" we define as # of pieces flipped by the move
                // We'll pick the move that flips the maximum pieces
                // We'll reuse actual_flips(...) to see how many pieces it flips
                Bitboard cur_side = turn == BLACK ? boardW : boardB;
                Bitboard alt_side = turn == WHITE ? boardW : boardB;

                int bestMove = -1;
                int bestCount = -1;
                for (Square move : valids) {
                    Bitboard flips = actual_flips(move, cur_side, alt_side);
                    int flipCount = popcount(flips); // from  'misc.h' or c++ builtin
                    if (flipCount > bestCount) {
                        bestCount = flipCount;
                        bestMove = move;
                    }
                }
                if (bestMove != -1) {
                    /*               std::cout << "[KaneFSM] Chose highest-flip move: " << bestMove
                                       << " flipping " << bestCount << " pieces.\n";*/
                    chosenMove = bestMove;
                }
                else {
                    // no moves found? Should not happen if valids is not empty
                    chosenMove = -1;
                }
                state = FSMState::DONE;
            } break;

            case FSMState::FALLBACK:
            default:
            {
                //std::cout << "[KaneFSM] No valid moves => pass.\n";
                chosenMove = -1;
                state = FSMState::DONE;
            } break;
            }
        }

        return chosenMove;
    }
}

