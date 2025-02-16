#include "../include/mcts.h"

//select the nodes to form a path down the tree
void mcts::selection(vnode * root)
{
    //select child based on UCT
    // ok we need to traverse down the nodes
    vnode * tmp_node = root;
    while(tmp_node != nullptr)
    {
        vnode * children = tmp_node->get_children();
        vnode * desired_node = nullptr;
        double best_uct_value = 0.0;
        while(children != nullptr)
        {
            //current children is the best so far
            double cur_uct_value = children->calc_uct();
            if(children->calc_uct() > best_uct_value )
            {
                best_uct_value = cur_uct_value;
                // set the desired node each time uct updated
                desired_node = children;
            }
            children = children->get_next_sibling();

        }

        //goes to the next layer of tree for the selected children
        tmp_node = desired_node;
    }

}

void mcts::expansion()
{
}

void mcts::simulation()
{
}

void mcts::backup()
{
}