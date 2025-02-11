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
#include <gtest/gtest.h>
#include "log.h"
#include "log_impl.h"
#include "log_output.h"

TEST(Log, AllLevel)
{
    LogOutput_Enable();
    LogFatal("fatal");
    LogErr("err");
    LogWarn("warn");
    LogNotice("notice");
    LogImportant("important");
    LogInfo("info");
    LogDbg("debug");
    LogTrace("trace");
    LogUndo();
    LogTag();
    LogOutput_Disable();
}

TEST(Log, Format)
{
    LogOutput_Enable();
    LogInfo("%s, %d, %f", "hello", 123456, 12.345);
    LogInfo("%d, %f, %s", 123456, 12.345, "world");
    LogOutput_Disable();
}

TEST(Log, LogPuts)
{
    LogOutput_Enable();
    LogPuts(LOG_LEVEL_INFO, "should be raw: %s, %d, %f");
    LogOutput_Disable();
}

TEST(Log, error)
{
    LogOutput_Enable();
    LogErrno(1, "");
    LogErrno(1, "no value");
    LogErrno(1, "has value:%d", 123);
    LogOutput_Disable();
}

TEST(Log, Truncate)
{
    LogOutput_Enable();

    std::string long_str(1025, 'l');
    std::string normal_str(1024, 'n');

    auto origin_len = LogSetMaxLength(1024);

    LogInfo(normal_str.c_str());
    LogNotice(long_str.c_str());
    LogInfo("%s", normal_str.c_str());
    LogNotice("%s", long_str.c_str());

    LogSetMaxLength(origin_len);
    LogOutput_Disable();
}
