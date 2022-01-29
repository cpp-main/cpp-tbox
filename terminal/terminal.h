#ifndef TBOX_TELNETD_TERMINAL_H_20220128
#define TBOX_TELNETD_TERMINAL_H_20220128

#include "interface.h"

namespace tbox::terminal {

class Terminal : public TerminalInteract,
                 public TerminalBuild {
  public:
    Terminal();
    ~Terminal();

  public:
    SessionToken newSession(Connection *wp_conn) override;
    bool deleteSession(const SessionToken &session) override;
    bool input(const SessionToken &session, const std::string &str) override;

  public:
    NodeToken create(const EndNode &info) override;
    NodeToken create(const DirNode &info) override;
    NodeToken root() const override;
    NodeToken find(const std::string &path) const override;
    bool mount(const NodeToken &parent, const NodeToken &child, const std::string &name) override;

  private:
    class Impl;
    Impl *impl_ = nullptr;
};

}

#endif //TBOX_TELNETD_TERMINAL_H_20220128
