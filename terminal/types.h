#ifndef TBOX_TERMINAL_TYPES_H_20220128
#define TBOX_TERMINAL_TYPES_H_20220128

#include <string>
#include <functional>
#include <tbox/base/cabinet.hpp>

namespace tbox::terminal {

class Session;

using SessionToken = cabinet::Token;
using NodeToken    = cabinet::Token;

using Args = std::vector<std::string>;
using Func = std::function<bool (Session &, const Args &)>;

enum class NodeType { kFunc, kDir };

struct NodeInfo {
    std::string name;
    NodeToken   token;

    NodeInfo(const std::string &n, const NodeToken &t) :
        name(n), token(t) { }
};

struct FuncInfo {
    std::string name;
    Func        func;   //!< 执行函数
    std::string help;   //!< 帮助说明

    FuncInfo(const std::string &n, const Func &f, const std::string &h) :
        name(n), func(f), help(h) { }
};

struct DirInfo {
    std::string name;

    DirInfo(const std::string &n) : name(n) { }
};

}

#endif //TBOX_TERMINAL_TYPES_H_20220128
