#ifndef TBOX_TERMINAL_NODE_H_20220207
#define TBOX_TERMINAL_NODE_H_20220207

#include "../types.h"

namespace tbox::terminal {

class Node {
  public:
    explicit Node(const std::string &name) : name_(name) { }
    virtual ~Node();

    virtual NodeType type() const = 0;

    inline std::string name() const { return name_; }

  private:
    std::string name_;
};

}

#endif //TBOX_TERMINAL_NODE_H_20220207
