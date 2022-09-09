#include <gtest/gtest.h>
#include <tbox/base/json.hpp>
#include "json.h"

using namespace tbox::util;
using namespace tbox;

TEST(Json, GetBoolField)
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

TEST(Json, GetUintField)
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

TEST(Json, GetIntField)
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

TEST(Json, GetDoubleField)
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

TEST(Json, GetStringField)
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

TEST(Json, HasField)
{
  Json js = R"(
{
  "object": {
    "a": 12,
    "b": 3
  },
  "array": [ 12, 3 ],
  "int": 12,
  "string", "hello"
})"_json;

  EXPECT_TRUE(json::HasObjectField(js, "object"));
  EXPECT_FALSE(json::HasObjectField(js, "array"));
  EXPECT_FALSE(json::HasObjectField(js, "int"));
  EXPECT_FALSE(json::HasObjectField(js, "string"));

  EXPECT_FALSE(json::HasArrayField(js, "object"));
  EXPECT_TRUE(json::HasArrayField(js, "array"));
  EXPECT_FALSE(json::HasArrayField(js, "int"));
  EXPECT_FALSE(json::HasArrayField(js, "string"));
}
