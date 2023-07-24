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
 * Copyright (c) 2018 Hevake and contributors, all rights reserved.
 *
 * This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
 * Use of this source code is governed by MIT license that can be found
 * in the LICENSE file in the root of the source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */
#ifndef TBOX_TERMINAL_NODE_H_20220207
#define TBOX_TERMINAL_NODE_H_20220207

#include "inner_types.h"

namespace tbox {
namespace terminal {

class Node {
  public:
    explicit Node(const std::string &help) : help_(help) { }
    virtual ~Node() { }

    virtual NodeType type() const = 0;
    std::string help() const { return help_; }

  private:
    std::string help_;
};

}
}

#endif //TBOX_TERMINAL_NODE_H_20220207
