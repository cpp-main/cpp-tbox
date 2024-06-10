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
/**
 * 在 recorder.h 的基础上加 ENABLE_TRACE_RECORDER 宏开关
 * 使 recorder 功能可以在编译期间被关彻底关闭
 */
#ifndef TBOX_TRACE_WRAPPED_RECORDER_H_20240610
#define TBOX_TRACE_WRAPPED_RECORDER_H_20240610

#if ENABLE_TRACE_RECORDER
    #include "recorder.h"
#else
    #define RECORD_SCOPE()
    #define RECORD_DEFINE(name)
    #define RECORD_START(name)
    #define RECORD_STOP(name)
    #define RECORD_EVENT()
#endif

#endif //TBOX_TRACE_WRAPPED_RECORDER_H_20240610
