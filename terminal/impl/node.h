#ifndef TBOX_TERMINAL_NODE_H_20220207
#define TBOX_TERMINAL_NODE_H_20220207

#include "inner_types.h"

namespace tbox::terminal {

class Node {
  public:
    explicit Node(const std::string &help) : help_(help) { }
    virtual ~Node() { }

    virtual NodeType type() const = 0;
    std::string help() const { return help_; }

  private:
    std::string help_;
};

}

#endif //TBOX_TERMINAL_NODE_H_20220207
