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

#ifndef TBOX_TERNIMAL_ADD_FUNC_NODE_H_20240107
#define TBOX_TERNIMAL_ADD_FUNC_NODE_H_20240107

#include <tbox/base/func_types.h>
#include "terminal_nodes.h"

namespace tbox {
namespace terminal {

NodeToken AddFuncNode(TerminalNodes &terminal, NodeToken parent_node,
                      const std::string &name, VoidFunc &&func);

NodeToken AddFuncNode(TerminalNodes &terminal, NodeToken parent_node,
                      const std::string &name, bool &value);

NodeToken AddFuncNode(TerminalNodes &terminal, NodeToken parent_node,
                      const std::string &name, std::string &value);

NodeToken AddFuncNode(TerminalNodes &terminal, NodeToken parent_node,
                      const std::string &name, int &value,
                      int min_value, int max_value);

NodeToken AddFuncNode(TerminalNodes &terminal, NodeToken parent_node,
                      const std::string &name, double &value,
                      double min_value, double max_value);

}
}

#endif //TBOX_TERNIMAL_ADD_FUNC_NODE_H_20240107
