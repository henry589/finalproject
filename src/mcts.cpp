#include "../include/mcts.h"
#include "../include/misc.h"
#include <iostream>
#include "../include/bitBoard.h"

using namespace bitboard;
//select the nodes to form a path down the tree
vnode * mcts::selection(vnode * root)
{
    //select child based on UCT
    // ok we need to traverse down the nodes
    vnode * curr_node = root;
    while(curr_node != nullptr)
    {
        vnode * best_child = nullptr;
        double best_uct_value = 0.0;
        vnode * children = curr_node->get_children();

        //means a leaf node is reached, has a potential child?
        if(children == nullptr)
        {
            //leaf node reached
            std::cout<<"\nselected leaf:"<<curr_node->boardB<<std::endl;
            return curr_node;
        }
        while(children != nullptr)
        {
            //current children is the best so far
            double cur_uct_value = children->calc_uct();
            if(cur_uct_value > best_uct_value )
            {
                best_uct_value = cur_uct_value;
                // set the desired node each time uct updated
                best_child = children;
            }
            
            // means this an unexplored child
            if(children->sim_visits == 0)
            {
                // perform random choice whether to take this children or not
                if(random_bool(0.5))
                {
                    best_child = children;
                }  

            }
            
            children = children->get_next_sibling();            
        }
        // std::cout<<"\ndsrd_n:"<<desired_node->boardB<<"\n";
        //goes to the next layer of tree for the selected children
        // std::cout<<"\ndesired node:"<<desired_node->boardB<<"\n";
        curr_node = best_child;
    }
    // return a nullptr if no selection could be made
    return nullptr;
}

vnode * mcts::expansion(vnode * lfnode)
{
    if(isTerminal(lfnode))
    {
        // do backup
    }
    else
    {
        //create valid children from current lfnode
        int valid_child = 0;
        vnode * children = createValidChildren(lfnode, valid_child);
        vnode * tmp_node = children;
        int nonce = getRandomNumber(0, valid_child-1);
        int count = 0;

        // if there's some valid children
        while(tmp_node != nullptr)
        {
            // return the random child
            if(nonce == count)
            {    
                return tmp_node;
            }
            tmp_node = tmp_node->get_next_sibling();
            ++count;
        }
    }

    return nullptr;
}

void mcts::simulation()
{
}

void mcts::backup()
{
}

bool mcts::isTerminal(vnode *node)
{
    return false;
}

uint64_t mcts::placeMove(Bitboard &board, int bit_pos)
{
    return board | (1 << bit_pos);
}

void mcts::boardViewer(const bitboard::Bitboard & boardB, const bitboard::Bitboard & boardW)
{
    //decode the bitboard
    uint64_t boardBtmp = boardB;
    uint64_t boardWtmp = boardW;
    int n = 0;
    char board_char_arr[64] = {};
    for(int m =0; m < 64; ++m)
    {
        board_char_arr[n++] = boardBtmp & 1 ? 'x' : boardWtmp & 1 ? 'o' : '-';
        boardBtmp >>= 1;
        boardWtmp >>= 1;
    }
    std::cout<<"\npretty board:\n";

    for (int m = 7; m >= 0; --m)
    {
        for(int i = (8 * m) + 7; i >= 8 * m; --i)
        {

            std::cout<<board_char_arr[i];
            if(i % 8 == 0)
            std::cout<<std::endl;
        }
    }
    std::cout<<"\nend board\n";

}

vnode * mcts::createValidChildren(vnode *node, int &child_count)
{
    // take current node and then check for all possible nodes
    // this will follow the othello rule
    const Bitboard & cur_boardB = node->boardB;
    const Bitboard & cur_boardW = node->boardW;
    const Bitboard & cur_side = node->turn == BLACK? cur_boardB : cur_boardW;
    const Bitboard & alt_side = node->turn == WHITE? cur_boardB : cur_boardW;
    const Bitboard & overlapped_board = cur_boardB | cur_boardW; 
    // assume the square sq is empty
    // we have to know whether it is empty square first
    for(Square sq = SQ_A1; sq <= SQ_H8; ++sq)
    {
        // check if not empty empty
        if( overlapped_board & (1ULL << sq))
        {
            continue;
        }
        else{ // empty
            // actual flips are the flips that would be done by the cur_boardB only
            const Bitboard & future_flips = actual_flips(sq, cur_side, alt_side);
       
            if(future_flips ^ 0) // future_flips contain something also means not equal to zero
            {
                const Bitboard & mod_cur_boardB = node->turn == BLACK? future_flips | cur_boardB | (1ULL<<sq) : ~future_flips & cur_boardB;
                const Bitboard & mod_cur_boardW = node->turn == WHITE? future_flips | cur_boardW | (1ULL<<sq) : ~future_flips & cur_boardW;
                node->append_child(mod_cur_boardB, mod_cur_boardW, !node->turn, sq);
                ++child_count;
            }
            
        }
    }
   

    // Bitboard 
    return node->get_children();
}