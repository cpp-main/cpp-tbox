#ifndef TBOX_TERMINAL_INNER_TYPES_H_20220214
#define TBOX_TERMINAL_INNER_TYPES_H_20220214

#include "../types.h"

namespace tbox {
namespace terminal {

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
}

#endif //TBOX_TERMINAL_INNER_TYPES_H_20220214
