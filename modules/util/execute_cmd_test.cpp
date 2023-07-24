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
#include "execute_cmd.h"
#include <gtest/gtest.h>
#include <iostream>
#include "fs.h"

namespace tbox {
namespace util {
namespace {

TEST(ExecuteCmd, WriteFile) {
    EXPECT_TRUE(ExecuteCmd(R"(echo -n "hello" > /tmp/test.txt)"));
    std::string rcontent;
    util::fs::ReadStringFromTextFile("/tmp/test.txt", rcontent);
    EXPECT_EQ(rcontent, "hello");
    util::fs::RemoveFile("/tmp/test.txt");
}

TEST(ExecuteCmd, WrongCmd) {
    EXPECT_FALSE(ExecuteCmd(R"(this_cmd_not_exit x y x)"));

    std::string result;
    EXPECT_FALSE(ExecuteCmd(R"(this_cmd_not_exit x y x)", result));
}

TEST(ExecuteCmd, ReadFile) {
    ASSERT_TRUE(util::fs::WriteStringToTextFile("/tmp/test.txt", "hello"));
    std::string result;
    EXPECT_TRUE(ExecuteCmd(R"(cat /tmp/test.txt)", result));
    EXPECT_EQ(result, "hello");
    util::fs::RemoveFile("/tmp/test.txt");
}

TEST(ExecuteCmd, ReadBigFile) {
    std::string wcontent(4096, 'x');
    ASSERT_TRUE(util::fs::WriteStringToTextFile("/tmp/test.txt", wcontent));
    std::string result;
    EXPECT_TRUE(ExecuteCmd(R"(cat /tmp/test.txt)", result));
    EXPECT_EQ(result, wcontent);
    util::fs::RemoveFile("/tmp/test.txt");
}

}
}
}
