#ifndef TBOX_TELNETD_SHELL_H_20220128
#define TBOX_TELNETD_SHELL_H_20220128

#include "interface.h"

namespace tbox::shell {

class Shell : public ShellInteract,
              public ShellBuild {
  public:
    Shell();
    ~Shell();

  public:
    Session newSession(Connection *wp_conn) override;
    bool deleteSession(const Session &session) override;
    bool input(const Session &session, const std::string &str) override;

  public:
    Node create(const EndNode &info) override;
    Node create(const DirNode &info) override;
    Node root() const override;
    Node find(const std::string &path) const override;
    bool mount(const Node &parent, const Node &child, const std::string &name) override;

  private:
    class Impl;
    Impl *impl_ = nullptr;
};

}

#endif //TBOX_TELNETD_SHELL_H_20220128
