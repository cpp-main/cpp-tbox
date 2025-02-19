/*
 *     .============.
 *    //  M A K E  / \
 *   //  C++ DEV  /   \
 *  //  E A S Y  /  \/ \
 * ++ ----------.  \/\  .
 *  \\     \     \ /\  /
 *   \\     \     \   /
 *    \\     \     \ /
 *     -============'
 *
 * Copyright (c) 2024 Hevake and contributors, all rights reserved.
 *
 * This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
 * Use of this source code is governed by MIT license that can be found
 * in the LICENSE file in the root of the source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */

#ifndef TBOX_TERNIMAL_HELPER_H_20240107
#define TBOX_TERNIMAL_HELPER_H_20240107

#include <limits>
#include <tbox/base/func_types.h>
#include "terminal_nodes.h"

namespace tbox {
namespace terminal {

//! 添加目标结点
NodeToken AddDirNode(TerminalNodes &terminal, NodeToken parent_node,
                     const std::string &name, const std::string &help = "");

//! 添加Boolean变量读写结点
NodeToken AddFuncNode(TerminalNodes &terminal, NodeToken parent_node,
                      const std::string &name, bool &value);

//! 添加字串变量读写结点
NodeToken AddFuncNode(TerminalNodes &terminal, NodeToken parent_node,
                      const std::string &name, std::string &value);

//! 添加整数变量读写结点
NodeToken AddFuncNode(TerminalNodes &terminal, NodeToken parent_node,
                      const std::string &name, int &value,
                      int min_value = std::numeric_limits<int>::min(),
                      int max_value = std::numeric_limits<int>::max());

//! 添加浮点数变量读写结点
NodeToken AddFuncNode(TerminalNodes &terminal, NodeToken parent_node,
                      const std::string &name, double &value,
                      double min_value = std::numeric_limits<double>::min(),
                      double max_value = std::numeric_limits<double>::max());

//! 添加函数调用结点
NodeToken AddFuncNode(TerminalNodes &terminal, NodeToken parent_node, const std::string &name, VoidFunc &&func);

//! 通用结点描述
struct FuncNodeProfile {
    std::string usage;  //!< 使用说明
    std::string help;   //!< 帮助
};

//! 字串读写结点描述
struct StringFuncNodeProfile : public FuncNodeProfile {
    std::function<std::string()> get_func;            //! 读取函数
    std::function<bool(const std::string&)> set_func; //! 设置函数
};
//! 添加字串读写结点
NodeToken AddFuncNode(TerminalNodes &terminal, NodeToken parent_node, const std::string &name, const StringFuncNodeProfile &profile);

//! Boolean读写结点描述
struct BooleanFuncNodeProfile : public FuncNodeProfile {
    std::function<bool()> get_func;     //! 读取函数
    std::function<bool(bool)> set_func; //! 设置函数
};
//! 添加Boolean读写结点
NodeToken AddFuncNode(TerminalNodes &terminal, NodeToken parent_node, const std::string &name, const BooleanFuncNodeProfile &profile);

//! 数值读写结点描述模板
template <typename T>
struct NumberFuncNodeProfile : public FuncNodeProfile {
    std::function<T()> get_func;      //! 读取函数
    std::function<bool(T)> set_func;  //! 设置函数
    T min_value = std::numeric_limits<T>::min();  //! 最小值
    T max_value = std::numeric_limits<T>::max();  //! 最大值
};

//! 整数读写结点描述模板
using IntegerFuncNodeProfile = NumberFuncNodeProfile<int>;
//! 添加整数读写结点
NodeToken AddFuncNode(TerminalNodes &terminal, NodeToken parent_node, const std::string &name, const IntegerFuncNodeProfile &profile);

//! 浮点数读写结点描述模板
using DoubleFuncNodeProfile = NumberFuncNodeProfile<double>;
//! 添加浮点数读写结点
NodeToken AddFuncNode(TerminalNodes &terminal, NodeToken parent_node, const std::string &name, const DoubleFuncNodeProfile &profile);

}
}

#endif //TBOX_TERNIMAL_HELPER_H_20240107
