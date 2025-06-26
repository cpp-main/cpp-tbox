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
 * Copyright (c) 2025 Hevake and contributors, all rights reserved.
 *
 * This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
 * Use of this source code is governed by MIT license that can be found
 * in the LICENSE file in the root of the source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */
#include <gtest/gtest.h>
#include "parse_as.h"

namespace tbox {
namespace util {

TEST(ParseAs, Bool)
{
    bool value = false;
    EXPECT_TRUE(ParseAs("yes", value));
    EXPECT_TRUE(value);

    value = false;
    EXPECT_TRUE(ParseAs("y", value));
    EXPECT_TRUE(value);

    value = false;
    EXPECT_TRUE(ParseAs("True", value));
    EXPECT_TRUE(value);

    value = false;
    EXPECT_TRUE(ParseAs("On", value));
    EXPECT_TRUE(value);

    value = true;
    EXPECT_TRUE(ParseAs("no", value));
    EXPECT_FALSE(value);

    value = true;
    EXPECT_TRUE(ParseAs("N", value));
    EXPECT_FALSE(value);

    value = true;
    EXPECT_TRUE(ParseAs("False", value));
    EXPECT_FALSE(value);

    EXPECT_FALSE(ParseAs("", value));
    EXPECT_FALSE(ParseAs("A", value));
    EXPECT_FALSE(ParseAs("343", value));
}

TEST(ParseAs, Int)
{
    int value = -1;
    EXPECT_TRUE(ParseAs("-10", value));
    EXPECT_EQ(value, -10);

    EXPECT_FALSE(ParseAs("", value));
    EXPECT_FALSE(ParseAs("A", value));
    EXPECT_FALSE(ParseAs("true", value));
}

TEST(ParseAs, UnsignedInt)
{
    unsigned int value = -1;
    EXPECT_TRUE(ParseAs("10", value));
    EXPECT_EQ(value, 10);

    EXPECT_TRUE(ParseAs("-12", value));
    EXPECT_EQ(value, -12);

    EXPECT_FALSE(ParseAs("", value));
}

TEST(ParseAs, long)
{
    long value = -1;
    EXPECT_TRUE(ParseAs("112233445566778899", value));
    EXPECT_EQ(value, 112233445566778899);

    EXPECT_FALSE(ParseAs("", value));
}

TEST(ParseAs, Double)
{
    double value = -1;
    EXPECT_TRUE(ParseAs("10.123", value));
    EXPECT_DOUBLE_EQ(value, 10.123);

    EXPECT_FALSE(ParseAs("", value));
    EXPECT_FALSE(ParseAs("A", value));
}

}
}
