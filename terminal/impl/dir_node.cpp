#include "dir_node.h"

namespace tbox::terminal {

bool DirNode::addChild(const NodeToken &nt, const std::string &child_name)
{
    auto iter = children_.find(child_name);
    if (iter != children_.end())
        return false;

    children_.insert(std::make_pair(child_name, nt));
    return true;
}

NodeToken DirNode::findChild(const std::string &child_name) const
{
    auto iter = children_.find(child_name);
    if (iter == children_.end())
        return NodeToken();
    return iter->second;
}

std::vector<NodeInfo> DirNode::children() const
{
    std::vector<NodeInfo> child_vec;
    for (auto item : children_) {
        NodeInfo info{ item.first, item.second };
        child_vec.push_back(info);
    }
    return child_vec;
}

}
