#ifndef TBOX_TERMINAL_INTERFACE_H_20220128
#define TBOX_TERMINAL_INTERFACE_H_20220128

#include "types.h"

namespace tbox::terminal {

class Connection {
  public:
    virtual bool send(const SessionToken &st, char ch) = 0;
    virtual bool send(const SessionToken &st, const std::string &str) = 0;
    virtual bool endSession(const SessionToken &st) = 0;
    virtual bool isValid(const SessionToken &st) const = 0;

  protected:
    virtual ~Connection() { }
};

class TerminalInteract {
  public:
    virtual SessionToken newSession(Connection *wp_conn) = 0;
    virtual bool deleteSession(const SessionToken &st) = 0;

    virtual bool onBegin(const SessionToken &st) = 0;
    virtual bool onExit(const SessionToken &st) = 0;
    virtual bool onRecvString(const SessionToken &st, const std::string &str) = 0;
    virtual bool onRecvWindowSize(const SessionToken &st, uint16_t w, uint16_t h) = 0;

  protected:
    virtual ~TerminalInteract() { }
};

class TerminalBuild {
  public:
    virtual NodeToken createFuncNode(const Func &func, const std::string &help) = 0;
    virtual NodeToken createDirNode() = 0;

    virtual NodeToken rootNode() const = 0;
    virtual NodeToken findNode(const std::string &path) const = 0;

    virtual bool mountNode(const NodeToken &parent, const NodeToken &child, const std::string &name) = 0;

  protected:
    virtual ~TerminalBuild() { }
};

class Session {
  public:
    virtual bool send(char ch) = 0;
    virtual bool send(const std::string &str) = 0;
    virtual void endSession() = 0;
    virtual bool isValid() const = 0;

    virtual bool window_width() const = 0;
    virtual bool window_height() const = 0;

  protected:
    virtual ~Session() { }
};

}

#endif //TBOX_TERMINAL_INTERFACE_H_20220128
