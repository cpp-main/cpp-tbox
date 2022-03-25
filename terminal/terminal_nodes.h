#ifndef TBOX_TERMINAL_NODES_H_20220214
#define TBOX_TERMINAL_NODES_H_20220214

#include "types.h"

namespace tbox {
namespace terminal {

class TerminalNodes {
  public:
    virtual NodeToken createFuncNode(const Func &func, const std::string &help = "") = 0;
    virtual NodeToken createDirNode(const std::string &help = "") = 0;

    virtual NodeToken rootNode() const = 0;
    virtual NodeToken findNode(const std::string &path) const = 0;

    virtual bool mountNode(const NodeToken &parent, const NodeToken &child, const std::string &name) = 0;

  protected:
    virtual ~TerminalNodes() { }
};

}
}

#endif //TBOX_TERMINAL_NODES_H_20220214
