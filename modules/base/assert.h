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
/**
 * 定义宏 TBOX_ASSERT(), 令其它错误信息打印到Log日志上
 */
#ifndef TBOX_BASE_ASSERT_H_20221026
#define TBOX_BASE_ASSERT_H_20221026

#include <cstdlib>
#include "log.h"

//! WARN: Don't use this in log related module

#ifdef  NDEBUG //! 在非调试模式下，什么都不用做
# define TBOX_ASSERT(expr) void(0)
#else //! 在调试模式下，要在日志中打印错误并退出
# define TBOX_ASSERT(expr) \
  if (!(expr)) { \
    LogFatal("TBOX_ASSERT(%s)", #expr); \
    std::abort(); \
  }
#endif

#endif //TBOX_BASE_ASSERT_H_20221026
