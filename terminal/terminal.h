#ifndef TBOX_TELNETD_TERMINAL_H_20220128
#define TBOX_TELNETD_TERMINAL_H_20220128

#include "terminal_interact.h"
#include "terminal_nodes.h"

namespace tbox::terminal {

class Terminal : public TerminalInteract,
                 public TerminalNodes {
  public:
    Terminal();
    ~Terminal();

  public:
    SessionToken newSession(Connection *wp_conn) override;
    bool deleteSession(const SessionToken &st) override;

    bool onBegin(const SessionToken &st) override;
    bool onRecvString(const SessionToken &st, const std::string &str) override;
    bool onRecvWindowSize(const SessionToken &st, uint16_t w, uint16_t h) override;
    bool onExit(const SessionToken &st) override;

  public:
    NodeToken createFuncNode(const Func &func, const std::string &help) override;
    NodeToken createDirNode(const std::string &help) override;
    NodeToken rootNode() const override;
    NodeToken findNode(const std::string &path) const override;
    bool mountNode(const NodeToken &parent, const NodeToken &child, const std::string &name) override;

  private:
    class Impl;
    Impl *impl_ = nullptr;
};

}

#endif //TBOX_TELNETD_TERMINAL_H_20220128
