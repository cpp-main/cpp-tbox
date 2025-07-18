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
#include <tbox/base/json.hpp>

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

    value = false;
    EXPECT_TRUE(StringTo("1", value));
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

    value = true;
    EXPECT_TRUE(StringTo("0", value));
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

TEST(StringTo, uint16_t)
{
    uint16_t value = 0;
    EXPECT_TRUE(StringTo("12345", value));
    EXPECT_EQ(value, 12345);

    EXPECT_TRUE(StringTo("0xFFFF", value, 16));
    EXPECT_EQ(value, 0xFFFF);

    EXPECT_FALSE(StringTo("0x10000", value, 16));
    EXPECT_FALSE(StringTo("65536", value));
}

TEST(StringTo, uint8_t)
{
    uint8_t value = 0;
    EXPECT_TRUE(StringTo("123", value));
    EXPECT_EQ(value, 123);

    EXPECT_TRUE(StringTo("0xFF", value, 16));
    EXPECT_EQ(value, 0xFF);

    EXPECT_FALSE(StringTo("0x100", value, 16));
    EXPECT_FALSE(StringTo("256", value));
}

TEST(StringTo, Double)
{
    double value = -1;
    EXPECT_TRUE(StringTo("10.123", value));
    EXPECT_DOUBLE_EQ(value, 10.123);

    EXPECT_FALSE(StringTo("", value));
    EXPECT_FALSE(StringTo("A", value));
}

TEST(StringTo, Json)
{
    Json js;
    EXPECT_TRUE(StringTo(R"({})", js));
    EXPECT_TRUE(StringTo(R"(123)", js));

    EXPECT_TRUE(StringTo(R"({"a":123,"b":"test"})", js));
    EXPECT_EQ(js["a"].get<int>(), 123);
    EXPECT_EQ(js["b"].get<std::string>(), "test");

    EXPECT_FALSE(StringTo(R"()", js));
    EXPECT_FALSE(StringTo(R"([})", js));
}

}
}
