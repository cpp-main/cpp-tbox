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
#ifndef TBOX_TERMINAL_INNER_TYPES_H_20220214
#define TBOX_TERMINAL_INNER_TYPES_H_20220214

#include "../types.h"

namespace tbox {
namespace terminal {

enum class NodeType { kFunc, kDir };

struct NodeInfo {
    std::string name;
    NodeToken   token;

    NodeInfo(const std::string &n, const NodeToken &t) :
        name(n), token(t) { }
};

using PathItem  = std::pair<std::string, NodeToken>;
using Path      = std::vector<PathItem>;

}
}

#endif //TBOX_TERMINAL_INNER_TYPES_H_20220214
