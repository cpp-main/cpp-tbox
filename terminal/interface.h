#ifndef TBOX_TERMINAL_INTERFACE_H_20220128
#define TBOX_TERMINAL_INTERFACE_H_20220128

#include "types.h"

namespace tbox::terminal {

class Connection {
  public:
    virtual bool send(const SessionToken &session, const std::string &str) = 0;
    virtual bool endSession(const SessionToken &session) = 0;
    virtual bool isValid(const SessionToken &session) const = 0;

  protected:
    virtual ~Connection() { }
};

class TerminalInteract {
  public:
    virtual SessionToken newSession(Connection *wp_conn) = 0;
    virtual bool deleteSession(const SessionToken &session) = 0;
    virtual bool input(const SessionToken &session, const std::string &str) = 0;
    virtual bool windowSize(const SessionToken &st, uint16_t w, uint16_t h) = 0;

  protected:
    virtual ~TerminalInteract() { }
};

class TerminalBuild {
  public:
    virtual NodeToken create(const EndNode &info) = 0;
    virtual NodeToken create(const DirNode &info) = 0;

    virtual NodeToken root() const = 0;
    virtual NodeToken find(const std::string &path) const = 0;

    virtual bool mount(const NodeToken &parent, const NodeToken &child, const std::string &name) = 0;

  protected:
    virtual ~TerminalBuild() { }
};

class Session {
  public:
    virtual bool send(const std::string &str) = 0;
    virtual void endSession() = 0;
    virtual bool isValid() const = 0;

  protected:
    virtual ~Session() { }
};

}

#endif //TBOX_TERMINAL_INTERFACE_H_20220128
