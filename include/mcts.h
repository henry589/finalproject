// vanilla monte-carlo tree search
#ifndef MCTS_H
#define MCTS_H
#include "nodeManager.h"
class mcts{
    public:
    vnode * currentTree;
    vnode * selection(vnode * root);
    vnode * expansion(vnode * lfnode);
    void simulation();
    void backup();
    bool isTerminal(vnode * node);
    vnode * createValidChildren(vnode * node,  int &child_count);
};

#endif