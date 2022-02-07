#ifndef TBOX_TERMINAL_DIR_NODE_H_20220207
#define TBOX_TERMINAL_DIR_NODE_H_20220207

#include "node.h"
#include <map>

namespace tbox::terminal {

class DirNode : public Node {
  public:
    using Node::Node;

    NodeType type() const override { return NodeType::kDir; }

    bool addChild(const NodeToken &nt, const std::string &child_name);
    NodeToken findChild(const std::string &child_name) const;
    std::vector<NodeInfo> children() const;

  private:
    std::map<std::string, NodeToken> children_;
};

}

#endif //TBOX_TERMINAL_DIR_NODE_H_20220207
