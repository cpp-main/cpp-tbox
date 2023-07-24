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

#include <tbox/base/defines.h>
#include "argument_parser.h"

using namespace tbox::util;

TEST(ArgumentParser, Fail)
{
    const char* argv[] = {"test_fail", "-x"};
    ArgumentParser parser(
        [&](char short_opt, const std::string &long_opt, ArgumentParser::OptionValue& opt_value) {
            (void)short_opt;
            (void)long_opt;
            (void)opt_value;
            return false;
        }
    );

    ASSERT_FALSE(parser.parse(NUMBER_OF_ARRAY(argv), argv));
}

TEST(ArgumentParser, LongOptionKeyAndValue)
{
    const char* argv[] = {"test_long_opt_key_and_value", "--key=123", "-n"};
    ArgumentParser parser(
        [&](char short_opt, const std::string &long_opt, ArgumentParser::OptionValue& opt_value) {
            if (short_opt == 0) {
                EXPECT_EQ(long_opt, "key");
                EXPECT_TRUE(opt_value.valid());
                EXPECT_EQ(opt_value.get(), "123");
            } else {
                EXPECT_EQ(short_opt, 'n');
                EXPECT_FALSE(opt_value.valid());
            }
            return true;
        }
    );
    ASSERT_TRUE(parser.parse(NUMBER_OF_ARRAY(argv), argv));
}

TEST(ArgumentParser, LongOptionKeyAndValueWithQuot)
{
    const char* argv[] = {"any", R"(--name='Hevake Lee')", R"(--address="Shenzhen China")"};
    bool has_name = false;
    bool has_address = false;
    ArgumentParser parser(
        [&](char short_opt, const std::string &long_opt, ArgumentParser::OptionValue& opt_value) {
            if (long_opt == "name") {
                EXPECT_TRUE(opt_value.valid());
                EXPECT_EQ(opt_value.get(), "Hevake Lee");
                has_name = true;
            } else if (long_opt == "address"){
                EXPECT_TRUE(opt_value.valid());
                EXPECT_EQ(opt_value.get(), "Shenzhen China");
                has_address = true;
            }
            (void)short_opt;
            return true;
        }
    );
    ASSERT_TRUE(parser.parse(NUMBER_OF_ARRAY(argv), argv));
    EXPECT_TRUE(has_name);
    EXPECT_TRUE(has_address);
}

TEST(ArgumentParser, All)
{
    const char* argv[] = {"test_all", "-ab", "123", "-x", "--set", "xyz", "--run", "--key=value"};
    ArgumentParser parser(
        [&](char short_opt, const std::string &long_opt, ArgumentParser::OptionValue& opt_value) {
            if (short_opt == 'a') {
                EXPECT_FALSE(opt_value.valid());
            } else if (short_opt == 'b') {
                EXPECT_TRUE(opt_value.valid());
                EXPECT_EQ(opt_value.get(), "123");
            } else if (short_opt == 'x') {
                EXPECT_TRUE(opt_value.valid());
            } else if (long_opt == "set") {
                EXPECT_TRUE(opt_value.valid());
                EXPECT_EQ(opt_value.get(), "xyz");
            } else if (long_opt == "run") {
                EXPECT_TRUE(opt_value.valid());
            } else if (long_opt == "key") {
                EXPECT_TRUE(opt_value.valid());
                EXPECT_EQ(opt_value.get(), "value");
            } else {
                return false;
            }
            return true;
        }
    );

    ASSERT_TRUE(parser.parse(NUMBER_OF_ARRAY(argv), argv));
}

