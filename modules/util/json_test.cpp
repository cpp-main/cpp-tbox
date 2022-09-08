#include <gtest/gtest.h>
#include <tbox/base/json.hpp>
#include "json.h"

using namespace tbox::util;
using namespace tbox;

TEST(Json, bool)
{
    Json js;
    js["test"] = "true"_json;

    bool b_value = false;
    EXPECT_TRUE(json::GetField(js, "test", b_value));
    EXPECT_TRUE(b_value);

    int i_value = 0;
    EXPECT_FALSE(json::GetField(js, "test", i_value));

    double d_value = 0;
    EXPECT_FALSE(json::GetField(js, "test", d_value));

    std::string s_value;
    EXPECT_FALSE(json::GetField(js, "test", s_value));
}

TEST(Json, uint)
{
    Json js;
    js["test"] = "10"_json;

    bool b_value = false;
    EXPECT_FALSE(json::GetField(js, "test", b_value));

    unsigned int u_value = 0;
    EXPECT_TRUE(json::GetField(js, "test", u_value));
    EXPECT_EQ(u_value, 10);

    int i_value = 0;
    EXPECT_TRUE(json::GetField(js, "test", i_value));
    EXPECT_EQ(i_value, 10);

    double d_value = 0;
    EXPECT_TRUE(json::GetField(js, "test", d_value));
    EXPECT_FLOAT_EQ(d_value, 10.0);

    std::string s_value;
    EXPECT_FALSE(json::GetField(js, "test", s_value));
}

TEST(Json, int)
{
    Json js;
    js["test"] = "-10"_json;

    bool b_value = false;
    EXPECT_FALSE(json::GetField(js, "test", b_value));

    unsigned int u_value = 0;
    EXPECT_FALSE(json::GetField(js, "test", u_value));

    int i_value = 0;
    EXPECT_TRUE(json::GetField(js, "test", i_value));
    EXPECT_EQ(i_value, -10);

    double d_value = 0;
    EXPECT_TRUE(json::GetField(js, "test", d_value));
    EXPECT_FLOAT_EQ(d_value, -10.0);

    std::string s_value;
    EXPECT_FALSE(json::GetField(js, "test", s_value));
}

TEST(Json, double)
{
    Json js;
    js["test"] = 1.234;

    bool b_value = false;
    EXPECT_FALSE(json::GetField(js, "test", b_value));

    unsigned int u_value = 0;
    EXPECT_FALSE(json::GetField(js, "test", u_value));

    int i_value = 0;
    EXPECT_FALSE(json::GetField(js, "test", i_value));

    double d_value = 0;
    EXPECT_TRUE(json::GetField(js, "test", d_value));
    EXPECT_FLOAT_EQ(d_value, 1.234);

    std::string s_value;
    EXPECT_FALSE(json::GetField(js, "test", s_value));
}

TEST(Json, string)
{
    Json js;
    js["test"] = "hello";

    bool b_value = false;
    EXPECT_FALSE(json::GetField(js, "test", b_value));

    unsigned int u_value = 0;
    EXPECT_FALSE(json::GetField(js, "test", u_value));

    int i_value = 0;
    EXPECT_FALSE(json::GetField(js, "test", i_value));

    std::string s_value;
    EXPECT_TRUE(json::GetField(js, "test", s_value));
    EXPECT_EQ(s_value, "hello");

    double d_value = 0;
    EXPECT_FALSE(json::GetField(js, "test", d_value));
}
