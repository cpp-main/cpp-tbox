#ifndef TBOX_TERMINAL_TYPES_H_20220128
#define TBOX_TERMINAL_TYPES_H_20220128

#include <string>
#include <functional>
#include <tbox/base/cabinet.hpp>

namespace tbox::terminal {

class Session;

using SessionToken = cabinet::Token;
using NodeToken    = cabinet::Token;

using Args = std::vector<std::string>;
using Func = std::function<bool (const Session &s, const Args &)>;

enum class NodeType { kFunc, kDir };

struct NodeInfo {
    std::string name;
    NodeToken   token;

    NodeInfo(const std::string &n, const NodeToken &t) :
        name(n), token(t) { }
};

using PathItem  = std::pair<std::string, NodeToken>;
using Path      = std::vector<PathItem>;

}

#endif //TBOX_TERMINAL_TYPES_H_20220128
