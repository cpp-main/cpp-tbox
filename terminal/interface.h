#ifndef TBOX_TERMINAL_INTERFACE_H_20220128
#define TBOX_TERMINAL_INTERFACE_H_20220128

#include "types.h"

namespace tbox::terminal {

class Connection {
  public:
    virtual bool send(const SessionToken &session, const std::string &str) = 0;
    virtual bool endSession(const SessionToken &session) = 0;
    virtual bool isValid(const SessionToken &session) const = 0;
};

class TerminalInteract {
  public:
    virtual SessionToken newSession(Connection *wp_conn) = 0;
    virtual bool deleteSession(const SessionToken &session) = 0;
    virtual bool input(const SessionToken &session, const std::string &str) = 0;
};

class TerminalBuild {
  public:
    virtual NodeToken create(const EndNode &info) = 0;
    virtual NodeToken create(const DirNode &info) = 0;

    virtual NodeToken root() const = 0;
    virtual NodeToken find(const std::string &path) const = 0;

    virtual bool mount(const NodeToken &parent, const NodeToken &child, const std::string &name) = 0;
};

class Session {
  public:
    virtual bool send(const std::string &str);
    virtual void endSession();
    virtual bool isValid() const = 0;
};

}

#endif //TBOX_TERMINAL_INTERFACE_H_20220128
