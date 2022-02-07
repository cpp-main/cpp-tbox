#include "func_node.h"

namespace tbox::terminal {
FuncNode::FuncNode(const std::string &name,
                   const Func &func,
                   const std::string &help) :
    Node(name), func_(func), help_(help)
{ }

bool FuncNode::execute(Session &s, const Args &a) const
{
    if (func_)
        return func_(s, a);
    return false;
}

}
