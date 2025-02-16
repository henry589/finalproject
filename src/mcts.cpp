#include "../include/mcts.h"
#include "../include/misc.h"
#include <iostream>
//select the nodes to form a path down the tree
void mcts::selection(vnode * root)
{
    //select child based on UCT
    // ok we need to traverse down the nodes
    vnode * tmp_node = root;
    while(tmp_node != nullptr)
    {
                    std::cout<<"\nvxx";

        vnode * desired_node = nullptr;
        double best_uct_value = 0.0;
        vnode * children = tmp_node->get_children();
                            std::cout<<"\nvxx2";

        //means a leaf node is reached
        if(children == nullptr)
        {
            //leaf node reached
            std::cout<<"\nnode:"<<tmp_node->boardB<<"\n";
            return;
        }
        while(children != nullptr)
        {
            //current children is the best so far
            double cur_uct_value = children->calc_uct();
            if(cur_uct_value > best_uct_value )
            {
                best_uct_value = cur_uct_value;
                // set the desired node each time uct updated
                desired_node = children;
                std::cout<<"\nupdated children:"<<children->boardB<<"\n";
            }
                        // means this unexplored child
            if(children->sim_visits == 0)
            {
                // perform random choice whether to take this children or not
                if(random_bool(0.5))
                 {
                     desired_node = children;
                    std::cout<<"\nupdated rand children:"<<children->boardB<<"\n";
                    //  std::cout<<"\ntaken!\n";
                 }  

            }
            
            std::cout<<"\n next children:"<<children->boardB<<",best uct:"<<best_uct_value<<"curr uct:"<<cur_uct_value<<"\n";
            children = children->get_next_sibling();            
        }
        std::cout<<"\ndsrd_n:"<<desired_node->boardB<<"\n";
        //goes to the next layer of tree for the selected children
        // std::cout<<"\ndesired node:"<<desired_node->boardB<<"\n";
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