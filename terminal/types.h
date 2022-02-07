#ifndef TBOX_TERMINAL_TYPES_H_20220128
#define TBOX_TERMINAL_TYPES_H_20220128

#include <string>
#include <functional>
#include <tbox/base/cabinet.hpp>

namespace tbox::terminal {

class Session;

using SessionToken = cabinet::Token;
using NodeToken = cabinet::Token;

using Args = std::vector<std::string>;
using Func = std::function<void(Session &, const Args &)>;

struct EndNode {
    std::string name;
    Func func;          //!< 执行函数
    std::string help;   //!< 帮助说明
};

struct DirNode {
    std::string name;
    std::string passwd; //!< 访问密码
};

}

#endif //TBOX_TERMINAL_TYPES_H_20220128
