#ifndef TBOX_TELNETD_SHELL_H_20220128
#define TBOX_TELNETD_SHELL_H_20220128

#include <functional>
#include <string>
#include <tbox/base/cabinet.hpp>

namespace tbox::telnetd {

using Session = cabinet::Token;
using Node = cabinet::Token;
using Args = std::vector<std::string>;
using Func = std::function<void(const Args &)>;

struct EndNode {
    Func func;          //!< 执行函数
    std::string help;   //!< 帮助说明
};

struct DirNode {
    std::string passwd; //!< 访问密码
};


class Shell {
  public:
    Shell();
    ~Shell();

  public:
    using OutputFunc = std::function<void(const Session &, const std::string &)>;
    void setOutputFunc(const OutputFunc &func);

    Session newSession();
    bool deleteSession(const Session &session);

    bool input(const Session &session, const std::string &input);

  public:
    Node create(const EndNode &info);
    Node create(const DirNode &info);

    Node root() const;
    Node find(const std::string &path) const;

    bool mount(const Node &parent, const Node &child, const std::string &name);

  private:
    class Impl;
    Impl *impl_ = nullptr;
};

}

#endif //TBOX_TELNETD_SHELL_H_20220128
