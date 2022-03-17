#include "func_node.h"

namespace tbox::terminal {
FuncNode::FuncNode(const Func &func, const std::string &help) :
    Node(help), func_(func)
{ }

void FuncNode::execute(const Session &s, const Args &a) const
{
    if (func_)
        func_(s, a);
}

}
