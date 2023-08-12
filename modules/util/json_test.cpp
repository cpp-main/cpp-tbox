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
#include <tbox/base/json.hpp>
#include "json.h"

namespace tbox {
namespace util {
namespace json {

TEST(Json, GetBoolField)
{
    Json js;
    js["test"] = "true"_json;

    bool b_value = false;
    EXPECT_TRUE(GetField(js, "test", b_value));
    EXPECT_TRUE(b_value);

    int i_value = 0;
    EXPECT_FALSE(GetField(js, "test", i_value));

    double d_value = 0;
    EXPECT_FALSE(GetField(js, "test", d_value));

    std::string s_value;
    EXPECT_FALSE(GetField(js, "test", s_value));
}

TEST(Json, GetUintField)
{
    Json js;
    js["test"] = "10"_json;

    bool b_value = false;
    EXPECT_FALSE(GetField(js, "test", b_value));

    unsigned int u_value = 0;
    EXPECT_TRUE(GetField(js, "test", u_value));
    EXPECT_EQ(u_value, 10);

    int i_value = 0;
    EXPECT_TRUE(GetField(js, "test", i_value));
    EXPECT_EQ(i_value, 10);

    double d_value = 0;
    EXPECT_TRUE(GetField(js, "test", d_value));
    EXPECT_FLOAT_EQ(d_value, 10.0);

    std::string s_value;
    EXPECT_FALSE(GetField(js, "test", s_value));
}

TEST(Json, GetIntField)
{
    Json js;
    js["test"] = "-10"_json;

    bool b_value = false;
    EXPECT_FALSE(GetField(js, "test", b_value));

    unsigned int u_value = 0;
    EXPECT_FALSE(GetField(js, "test", u_value));

    int i_value = 0;
    EXPECT_TRUE(GetField(js, "test", i_value));
    EXPECT_EQ(i_value, -10);

    double d_value = 0;
    EXPECT_TRUE(GetField(js, "test", d_value));
    EXPECT_FLOAT_EQ(d_value, -10.0);

    std::string s_value;
    EXPECT_FALSE(GetField(js, "test", s_value));
}

TEST(Json, GetDoubleField)
{
    Json js;
    js["test"] = 1.234;

    bool b_value = false;
    EXPECT_FALSE(GetField(js, "test", b_value));

    unsigned int u_value = 0;
    EXPECT_FALSE(GetField(js, "test", u_value));

    int i_value = 0;
    EXPECT_FALSE(GetField(js, "test", i_value));

    double d_value = 0;
    EXPECT_TRUE(GetField(js, "test", d_value));
    EXPECT_FLOAT_EQ(d_value, 1.234);

    std::string s_value;
    EXPECT_FALSE(GetField(js, "test", s_value));
}

TEST(Json, GetStringField)
{
    Json js;
    js["test"] = "hello";

    bool b_value = false;
    EXPECT_FALSE(GetField(js, "test", b_value));

    unsigned int u_value = 0;
    EXPECT_FALSE(GetField(js, "test", u_value));

    int i_value = 0;
    EXPECT_FALSE(GetField(js, "test", i_value));

    std::string s_value;
    EXPECT_TRUE(GetField(js, "test", s_value));
    EXPECT_EQ(s_value, "hello");

    double d_value = 0;
    EXPECT_FALSE(GetField(js, "test", d_value));
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
  "string": "hello"
})"_json;

    EXPECT_TRUE(HasObjectField(js, "object"));
    EXPECT_FALSE(HasObjectField(js, "array"));
    EXPECT_FALSE(HasObjectField(js, "int"));
    EXPECT_FALSE(HasObjectField(js, "string"));

    EXPECT_FALSE(HasArrayField(js, "object"));
    EXPECT_TRUE(HasArrayField(js, "array"));
    EXPECT_FALSE(HasArrayField(js, "int"));
    EXPECT_FALSE(HasArrayField(js, "string"));
}

TEST(Json, FindEndPos) {
    EXPECT_EQ(FindEndPos("", 0), 0);
    EXPECT_EQ(FindEndPos(R"({})", 2), 2);   //! {}
    EXPECT_EQ(FindEndPos(R"([])", 2), 2);   //! []

    //! 测试[] {}嵌套
    EXPECT_EQ(FindEndPos(R"([12,{"name":"hevake"}])", 22), 22); //! [] 中嵌 {}
    EXPECT_EQ(FindEndPos(R"([12,{"name":"hevake}])", 21), 0);
    EXPECT_EQ(FindEndPos(R"([1,{"a":"}]"}])", 14), 14);
    EXPECT_EQ(FindEndPos(R"([1,{"a":"\"abc"}])", 17), 17);
    EXPECT_EQ(FindEndPos(R"({"a":1,b:[1,2,{"b":3}]})", 23), 23);
    EXPECT_EQ(FindEndPos(R"({)", 2), 0);

    //! 测试\转义符
    EXPECT_EQ(FindEndPos(R"("abc\"123")", 10), 10);
    EXPECT_EQ(FindEndPos(R"("abc\\"123")", 11), 7);
    EXPECT_EQ(FindEndPos(R"("abc\ \"123")", 12), 12);
    EXPECT_EQ(FindEndPos(R"("abc\\\\"123")", 13), 9);
    EXPECT_EQ(FindEndPos(R"("abc\ \\\"123")", 14), 14);

    //! 其它
    EXPECT_EQ(FindEndPos(R"( "abc" )", 7), 6);  //! 测试两边都有空格的情况
    EXPECT_EQ(FindEndPos(R"(    )", 4), 0);

    //! 异常情况
    EXPECT_EQ(FindEndPos(R"({])", 2), -1);
    EXPECT_EQ(FindEndPos(R"([})", 2), -1);
}

}
}
}
