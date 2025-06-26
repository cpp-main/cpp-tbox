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
#include "string_to.h"

namespace tbox {
namespace util {

TEST(StringTo, Bool)
{
    bool value = false;
    EXPECT_TRUE(StringTo("yes", value));
    EXPECT_TRUE(value);

    value = false;
    EXPECT_TRUE(StringTo("y", value));
    EXPECT_TRUE(value);

    value = false;
    EXPECT_TRUE(StringTo("True", value));
    EXPECT_TRUE(value);

    value = false;
    EXPECT_TRUE(StringTo("On", value));
    EXPECT_TRUE(value);

    value = true;
    EXPECT_TRUE(StringTo("no", value));
    EXPECT_FALSE(value);

    value = true;
    EXPECT_TRUE(StringTo("N", value));
    EXPECT_FALSE(value);

    value = true;
    EXPECT_TRUE(StringTo("False", value));
    EXPECT_FALSE(value);

    EXPECT_FALSE(StringTo("", value));
    EXPECT_FALSE(StringTo("A", value));
    EXPECT_FALSE(StringTo("343", value));
}

TEST(StringTo, Int)
{
    int value = 0;
    EXPECT_TRUE(StringTo("-10", value));
    EXPECT_EQ(value, -10);

    value = 0;
    EXPECT_TRUE(StringTo("1F", value, 16));
    EXPECT_EQ(value, 31);

    value = 0;
    EXPECT_FALSE(StringTo("1F", value));

    EXPECT_FALSE(StringTo("", value));
    EXPECT_FALSE(StringTo("12x", value));
    EXPECT_FALSE(StringTo("x12", value));
    EXPECT_FALSE(StringTo("A", value));
    EXPECT_FALSE(StringTo("true", value));
}

TEST(StringTo, UnsignedInt)
{
    unsigned int value = -1;
    EXPECT_TRUE(StringTo("10", value));
    EXPECT_EQ(value, 10);

    EXPECT_TRUE(StringTo("-12", value));
    EXPECT_EQ(value, -12);

    EXPECT_FALSE(StringTo("", value));
}

TEST(StringTo, long)
{
    long value = -1;
    EXPECT_TRUE(StringTo("112233445566778899", value));
    EXPECT_EQ(value, 112233445566778899);

    EXPECT_FALSE(StringTo("", value));
}

TEST(StringTo, Double)
{
    double value = -1;
    EXPECT_TRUE(StringTo("10.123", value));
    EXPECT_DOUBLE_EQ(value, 10.123);

    EXPECT_FALSE(StringTo("", value));
    EXPECT_FALSE(StringTo("A", value));
}

}
}
