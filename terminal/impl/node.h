#ifndef TBOX_TERMINAL_NODE_H_20220207
#define TBOX_TERMINAL_NODE_H_20220207

#include "../types.h"

namespace tbox::terminal {

class Node {
  public:
    virtual ~Node() { }
    virtual NodeType type() const = 0;
};

}

#endif //TBOX_TERMINAL_NODE_H_20220207
