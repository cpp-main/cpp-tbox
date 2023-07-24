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
#include <thread>
#include <gtest/gtest.h>
#include "pid_file.h"

using namespace tbox::util;

TEST(PidFile, base)
{
    const char *pid_file = "/tmp/test.pid";
    ::unlink(pid_file);
    {
        PidFile pid;
        ASSERT_TRUE(pid.lock(pid_file));

        EXPECT_EQ(::access(pid_file, R_OK), 0); //! 文件应该存在
    }

    EXPECT_EQ(::access(pid_file, R_OK), -1);    //! 文件应该打不开，因为已经删除了
    EXPECT_EQ(errno, ENOENT);
}
