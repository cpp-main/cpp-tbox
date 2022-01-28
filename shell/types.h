#ifndef TBOX_SHELL_TYPES_H_20220128
#define TBOX_SHELL_TYPES_H_20220128

#include <string>
#include <functional>
#include <tbox/base/cabinet.hpp>

namespace tbox::shell {

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

}

#endif //TBOX_SHELL_TYPES_H_20220128
