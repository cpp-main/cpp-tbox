#include "func_node.h"

namespace tbox::terminal {
FuncNode::FuncNode(const std::string &name,
                   const Func &func,
                   const std::string &help) :
    Node(name), func_(func), help_(help)
{ }

void FuncNode::execute(Session &s, const Args &a) const
{
    if (func_)
        func_(s, a);
}

}
