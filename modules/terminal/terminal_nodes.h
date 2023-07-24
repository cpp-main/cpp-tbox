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
#ifndef TBOX_TERMINAL_NODES_H_20220214
#define TBOX_TERMINAL_NODES_H_20220214

#include "types.h"
#include "session.h"

namespace tbox {
namespace terminal {

class TerminalNodes {
  public:
    virtual NodeToken createFuncNode(const Func &func, const std::string &help = "") = 0;
    virtual NodeToken createDirNode(const std::string &help = "") = 0;

    virtual NodeToken rootNode() const = 0;
    virtual NodeToken findNode(const std::string &path) const = 0;

    virtual bool mountNode(const NodeToken &parent, const NodeToken &child, const std::string &name) = 0;

  protected:
    virtual ~TerminalNodes() { }
};

}
}

#endif //TBOX_TERMINAL_NODES_H_20220214
