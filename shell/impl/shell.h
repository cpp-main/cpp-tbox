#ifndef TBOX_TELNETD_SHELL_IMPL_H_20220128
#define TBOX_TELNETD_SHELL_IMPL_H_20220128

#include "../shell.h"

namespace tbox::shell {

class Shell::Impl {
  public:
    Session newSession(Connection *wp_conn);
    bool deleteSession(const Session &session);
    bool input(const Session &session, const std::string &str);

  public:
    Node create(const EndNode &info);
    Node create(const DirNode &info);
    Node root() const;
    Node find(const std::string &path) const;
    bool mount(const Node &parent, const Node &child, const std::string &name);
};

}

#endif //TBOX_TELNETD_SHELL_IMPL_H_20220128
