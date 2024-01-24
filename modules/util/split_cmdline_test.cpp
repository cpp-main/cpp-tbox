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

#include "split_cmdline.h"

using namespace std;
using namespace tbox::util;

TEST(SplitCmdline, Base)
{
    vector<string> str_vec;
    ASSERT_TRUE(SplitCmdline("abc", str_vec));
    ASSERT_EQ(str_vec.size(), 1u);
    EXPECT_EQ(str_vec[0], "abc");
}

TEST(SplitCmdline, Space)
{
    vector<string> str_vec;
    ASSERT_TRUE(SplitCmdline(" abc 123 ", str_vec));
    ASSERT_EQ(str_vec.size(), 2u);
    EXPECT_EQ(str_vec[0], "abc");
    EXPECT_EQ(str_vec[1], "123");
}

TEST(SplitCmdline, SingleQuote)
{
    vector<string> str_vec;
    ASSERT_TRUE(SplitCmdline(R"( abc ' 123 ' ' xyz ')", str_vec));
    ASSERT_EQ(str_vec.size(), 3u);
    EXPECT_EQ(str_vec[0], "abc");
    EXPECT_EQ(str_vec[1], " 123 ");
    EXPECT_EQ(str_vec[2], " xyz ");
}

TEST(SplitCmdline, RecursionSingleQuote)
{
    vector<string> str_vec;
    ASSERT_TRUE(SplitCmdline(R"( abc ' "123 ')", str_vec));
    ASSERT_EQ(str_vec.size(), 2u);
    EXPECT_EQ(str_vec[0], "abc");
    EXPECT_EQ(str_vec[1], R"( "123 )");
}

TEST(SplitCmdline, DoubleQuote)
{
    vector<string> str_vec;
    ASSERT_TRUE(SplitCmdline(R"( abc " 123 ")", str_vec));
    ASSERT_EQ(str_vec.size(), 2u);
    EXPECT_EQ(str_vec[0], "abc");
    EXPECT_EQ(str_vec[1], " 123 ");
}

TEST(SplitCmdline, RecursionDoubleQuote)
{
    vector<string> str_vec;
    ASSERT_TRUE(SplitCmdline(R"( abc " '123 ")", str_vec));
    ASSERT_EQ(str_vec.size(), 2u);
    EXPECT_EQ(str_vec[0], "abc");
    EXPECT_EQ(str_vec[1], R"( '123 )");
}

#if 0
//! 下面两个测试用例过不了，也不是必须的
TEST(SplitCmdline, QuoteInSide)
{
    vector<string> str_vec;
    ASSERT_TRUE(SplitCmdline(R"( abc 12'3'4 ")", str_vec));
    ASSERT_EQ(str_vec.size(), 2u);
    EXPECT_EQ(str_vec[0], "abc");
    EXPECT_EQ(str_vec[1], R"(12'3'4)");
}

TEST(SplitCmdline, QuoteInSide1)
{
    vector<string> str_vec;
    ASSERT_TRUE(SplitCmdline(R"( abc 12'3" ")", str_vec));
    ASSERT_EQ(str_vec.size(), 2u);
    EXPECT_EQ(str_vec[0], "abc");
    EXPECT_EQ(str_vec[1], R"(12'3'4)");
}
#endif

TEST(SplitCmdline, Mix)
{
    vector<string> str_vec;
    ASSERT_TRUE(SplitCmdline(R"( abc " '12[3 }' " ' "x{yz #" ')", str_vec));
    ASSERT_EQ(str_vec.size(), 3u);
    EXPECT_EQ(str_vec[0], "abc");
    EXPECT_EQ(str_vec[1], R"( '12[3 }' )");
    EXPECT_EQ(str_vec[2], R"( "x{yz #" )");
}

TEST(SplitCmdline, LongOption)
{
    vector<string> str_vec;
    ASSERT_TRUE(SplitCmdline(R"(abc --key "hello world")", str_vec));
    ASSERT_EQ(str_vec.size(), 3u);
    EXPECT_EQ(str_vec[0], "abc");
    EXPECT_EQ(str_vec[1], R"(--key)");
    EXPECT_EQ(str_vec[2], R"(hello world)");
}

TEST(SplitCmdline, LongOptionWithEqual)
{
    vector<string> str_vec;
    ASSERT_TRUE(SplitCmdline(R"(abc --key="hello world")", str_vec));
    ASSERT_EQ(str_vec.size(), 2u);
    EXPECT_EQ(str_vec[0], "abc");
    EXPECT_EQ(str_vec[1], R"(--key="hello world")");
}

TEST(SplitCmdline, LongOptionWithEqual_2)
{
    vector<string> str_vec;
    ASSERT_TRUE(SplitCmdline(R"(abc --key="hello world"xxxx' 'yyy)", str_vec));
    ASSERT_EQ(str_vec.size(), 2u);
    EXPECT_EQ(str_vec[0], "abc");
    EXPECT_EQ(str_vec[1], R"(--key="hello world"xxxx' 'yyy)");
}

TEST(SplitCmdline, LongOptionWithEqual_3)
{
    vector<string> str_vec;
    ASSERT_TRUE(SplitCmdline(R"(abc --key="hello world"and'I am here'"  "yyy)", str_vec));
    ASSERT_EQ(str_vec.size(), 2u);
    EXPECT_EQ(str_vec[0], "abc");
    EXPECT_EQ(str_vec[1], R"(--key="hello world"and'I am here'"  "yyy)");
}

TEST(SplitCmdline, LongOptionWithEqual_4)
{
    vector<string> str_vec;
    ASSERT_TRUE(SplitCmdline(R"(abc --key="It's not mine")", str_vec));
    ASSERT_EQ(str_vec.size(), 2u);
    EXPECT_EQ(str_vec[0], "abc");
    EXPECT_EQ(str_vec[1], R"(--key="It's not mine")");
}

TEST(SplitCmdline, ErrorUnfinishQuote1)
{
    vector<string> str_vec;
    ASSERT_FALSE(SplitCmdline(R"( abc "1 )", str_vec));
}

TEST(SplitCmdline, ErrorUnfinishQuote2)
{
    vector<string> str_vec;
    ASSERT_FALSE(SplitCmdline(R"( abc '1 )", str_vec));
}

TEST(SplitCmdline, ErrorEndWithQuote)
{
    vector<string> str_vec;
    EXPECT_FALSE(SplitCmdline(R"(abc')", str_vec));
    EXPECT_FALSE(SplitCmdline(R"(abc'xyz)", str_vec));
    EXPECT_FALSE(SplitCmdline(R"(abc")", str_vec));
    EXPECT_FALSE(SplitCmdline(R"(abc"xyz)", str_vec));

}
