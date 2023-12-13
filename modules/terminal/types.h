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
#ifndef TBOX_TERMINAL_TYPES_H_20220128
#define TBOX_TERMINAL_TYPES_H_20220128

#include <cstdint>
#include <vector>
#include <string>
#include <functional>
#include <tbox/base/cabinet_token.h>

namespace tbox {
namespace terminal {

class Session;

using SessionToken = cabinet::Token;
using NodeToken    = cabinet::Token;

using Args = std::vector<std::string>;
using Func = std::function<void (const Session &s, const Args &)>;

}
}

#endif //TBOX_TERMINAL_TYPES_H_20220128
