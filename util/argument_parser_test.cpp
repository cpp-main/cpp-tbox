#include <gtest/gtest.h>

#include <tbox/base/defines.h>
#include "argument_parser.h"

using namespace tbox::util;

TEST(ArgumentParser, Fail)
{
    const char* argv[] = {"test", "-x"};
    ArgumentParser parser(
        [&](char short_opt, const std::string &long_opt, ArgumentParser::OptStr& opt_value) {
            return false;
        }
    );

    ASSERT_FALSE(parser.parse(NUMBER_OF_ARRAY(argv), argv));
}

TEST(ArgumentParser, All)
{
    const char* argv[] = {"test", "-ab", "123", "-x", "--set", "xyz", "--run"};
    ArgumentParser parser(
        [&](char short_opt, const std::string &long_opt, ArgumentParser::OptStr& opt_value) {
            if (short_opt == 'a') {
                EXPECT_TRUE(opt_value.isNull());
            } else if (short_opt == 'b') {
                EXPECT_FALSE(opt_value.isNull());
                EXPECT_EQ(opt_value.str(), "123");
            } else if (short_opt == 'x') {
                EXPECT_FALSE(opt_value.isNull());
            } else if (long_opt == "set") {
                EXPECT_FALSE(opt_value.isNull());
                EXPECT_EQ(opt_value.str(), "xyz");
            } else if (long_opt == "run") {
                EXPECT_TRUE(opt_value.isNull());
            }
            return true;
        }
    );

    ASSERT_TRUE(parser.parse(NUMBER_OF_ARRAY(argv), argv));
}

