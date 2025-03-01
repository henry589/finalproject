#include <iostream>
#include <sstream>
#include "../include/nodeManager.h"

    void vnode::remove_node(vnode * node)
    {
        if(node != nullptr && node->parent != nullptr)
        {
            vnode * ptr = node->parent->children_head;
            vnode * prev_node = nullptr;
            while(ptr != nullptr )
            {
                                    // std::cout<<"info:"<<ptr->boardB<<std::endl;

                if(ptr == node)
                {
                    // removal of first child
                    if(prev_node == nullptr)
                        node->parent->children_head = ptr->sibling_next;
                    else
                        prev_node -> sibling_next = ptr -> sibling_next;
                    delete(node);
                    std::cout<<"successfully remove child:"<<node->boardB<<",parent is:"<< node->parent->boardB<<std::endl;
                    break;
                }
                prev_node = ptr;
                ptr = ptr->sibling_next;
            }
        }
        else if(node->parent == nullptr) //when parent is null means it is a root node
        {
            //then just delete the node will do
                        std::cout<<"root point successfully removed:"<<node->boardB<<std::endl;

            delete(node);

        }
    }

    void * vnode::operator new(size_t size)
    {
        return pool.allocate();
    }

    void vnode::operator delete(void *p)
    {
        return pool.deallocate(p);
    }
    
    std::string vnode::get_dot_formatted(){
        return ss.str();
    }
    
    void vnode::traverse(vnode * node){
        ss<<std::endl<<"a"<<node->parent->boardB<<"->"<<"a"<<node->boardB<<";";

    }

    // breadth-first search for the tree
    void vnode::BFS(vnode * node, vnode::OpType method , bool include_current_node )
    {
        vnode * queue_exit = nullptr;
        vnode * enqueue_ptr = nullptr;
        vnode * dequeue_ptr = nullptr;
        vnode * tmp_node = node;
        void (*func)(vnode*) = nullptr;

        switch (method)
        {
            case vnode::OpType::TRAVERSE:
                /* code */
                func = traverse;
                break;
            
            case vnode::OpType::PRUNE:
                /* code */
                func = remove_node;
                break;
            
            default:
                func = traverse;
                break;
        }

        // see whether need to do something for current node 
        if(include_current_node)
        {
            func(node);  // function pointer to run
        }

        // int countDepth = 0;
        // breadth-first-search section
        while(tmp_node != nullptr)
        {
            // enqueue the children
            vnode * children = tmp_node->get_children();
            // if(children != nullptr) 
            // {
            //     countDepth++;
            //     std::cout<<"\ndepth:"<<countDepth<<"\n";
            // }
            while(children != nullptr)
            {
                // first element in queue
                if(queue_exit == nullptr)
                {
                    queue_exit = children;
                    queue_exit->bfs_next = nullptr;
                    enqueue_ptr = queue_exit;
                }
                else
                {
                    enqueue_ptr->bfs_next = children;
                    enqueue_ptr = children;
                    enqueue_ptr->bfs_next = nullptr;
                }
                // std::cout<<"chd:"<<children->boardB<<",p,"<<children->parent->boardB<<std::endl;

                children = children->sibling_next;
            }

       
            //deque stored node
            dequeue_ptr = queue_exit;

            // move the pointer to the next
            if(queue_exit != nullptr)
            {
                queue_exit = queue_exit->bfs_next;
                func(dequeue_ptr);  // function pointer to run
       
            }
            tmp_node = dequeue_ptr;
        }
        //done breadth-first-search

       
    }
    
    // append child to the children head, the link name is 'sibling_next', append both black and white bitboards
    void vnode::append_child(uint64_t boardB, uint64_t boardW, const bool & turn, uint8_t action)
    {
        vnode *child_node = new vnode();
        child_node->boardB = boardB;
        child_node->boardW = boardW;
        child_node->action_taken = action;
        child_node->turn = turn;
        child_node->sibling_next = children_head;
        child_node->parent = this;
        children_head = child_node; 
    }

    // get the current node uct value;
    double vnode::calc_uct()
    {
        if (sim_visits == 0) {
            return std::numeric_limits<double>::infinity(); // Encourage exploration
        }
        return (sim_reward / sim_visits) + explorationConstant * std::sqrt(std::log(parent->sim_visits) / sim_visits);   
    }
 
    // always point to the head of the children list
    vnode * vnode::get_children()
    {
        return children_head;
    }

    vnode * vnode::get_next_sibling()
    {
        return sibling_next;
    }

    vnode * vnode::get_parent()
    {
        return parent;
    }

