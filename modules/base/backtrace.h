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
#ifndef TBOX_BACKTRACE_H_20220708
#define TBOX_BACKTRACE_H_20220708

#include <string>
#include "log.h"

namespace tbox {

/// 导出调用栈
std::string DumpBacktrace(const unsigned int max_frames = 64);

}

#define LogBacktrace(level) LogPrintf((level), "call stack:\n%s", tbox::DumpBacktrace().c_str())

#endif // TBOX_BACKTRACE_H_20220708
