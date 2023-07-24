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
#ifndef TBOX_UTIL_SPLIT_CMDLINE_H_20220207
#define TBOX_UTIL_SPLIT_CMDLINE_H_20220207

#include <string>
#include <vector>

namespace tbox {
namespace util {

bool SplitCmdline(const std::string &cmd, std::vector<std::string> &args);

}
}

#endif //TBOX_UTIL_SPLIT_CMDLINE_H_20220207
