// vanilla monte-carlo tree search
#ifndef MCTS_H
#define MCTS_H
#include "nodeManager.h"
class mcts{
    public:
    vnode * currentTree;
    void selection(vnode * root);
    void expansion();
    void simulation();
    void backup();
};

#endif