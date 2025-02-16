#include "../include/mcts.h"
#include "../include/misc.h"
#include <iostream>
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
                        // means this unexplored child
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

vnode * mcts::createValidChildren(vnode *node, int &child_count)
{
}