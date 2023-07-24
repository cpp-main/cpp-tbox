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
#ifndef TBOX_TERMINAL_FUNC_NODE_H_20220207
#define TBOX_TERMINAL_FUNC_NODE_H_20220207

#include "node.h"
#include <map>

namespace tbox {
namespace terminal {

class FuncNode : public Node {
  public:
    FuncNode(const Func &func, const std::string &help);

    virtual NodeType type() const override { return NodeType::kFunc; }
    void execute(const Session &s, const Args &a) const;

  private:
    Func func_;
};

}
}

#endif //TBOX_TERMINAL_FUNC_NODE_H_20220207
