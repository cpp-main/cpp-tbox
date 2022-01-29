#ifndef TBOX_TERMINAL_INTERFACE_H_20220128
#define TBOX_TERMINAL_INTERFACE_H_20220128

#include "types.h"

namespace tbox::terminal {

class Connection {
  public:
    virtual bool send(const Session &session, const std::string &str) = 0;
    virtual bool endSession(const Session &session) = 0;
    virtual bool isValid(const Session &session) const = 0;
};

class TerminalInteract {
  public:
    virtual Session newSession(Connection *wp_conn) = 0;
    virtual bool deleteSession(const Session &session) = 0;
    virtual bool input(const Session &session, const std::string &str) = 0;
};

class TerminalBuild {
  public:
    virtual Node create(const EndNode &info) = 0;
    virtual Node create(const DirNode &info) = 0;

    virtual Node root() const = 0;
    virtual Node find(const std::string &path) const = 0;

    virtual bool mount(const Node &parent, const Node &child, const std::string &name) = 0;
};

}

#endif //TBOX_TERMINAL_INTERFACE_H_20220128
