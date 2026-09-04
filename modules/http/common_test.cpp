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
#include "common.h"
#include <sstream>

namespace tbox {
namespace http {
namespace {

using namespace std;

TEST(common, HttpVerToStream)
{
    EXPECT_EQ(HttpVerToString(HttpVer::k1_0), "HTTP/1.0");
    EXPECT_EQ(HttpVerToString(HttpVer::k1_1), "HTTP/1.1");
    EXPECT_EQ(HttpVerToString(HttpVer::k2_0), "HTTP/2.0");
}

TEST(common, MethodToStream)
{
    EXPECT_EQ(MethodToString(Method::kGet), "GET");
    EXPECT_EQ(MethodToString(Method::kHead), "HEAD");
    EXPECT_EQ(MethodToString(Method::kPut), "PUT");
    EXPECT_EQ(MethodToString(Method::kPost), "POST");
    EXPECT_EQ(MethodToString(Method::kDelete), "DELETE");
}

TEST(common, StateCodeToStream)
{
    EXPECT_EQ(StatusCodeToString(StatusCode::k200_OK), "200 OK");
    EXPECT_EQ(StatusCodeToString(StatusCode::k401_Unauthorized), "401 Unauthorized");
    EXPECT_EQ(StatusCodeToString(StatusCode::k404_NotFound), "404 Not Found");
    EXPECT_EQ(StatusCodeToString(StatusCode::k505_HTTPVersionNotSupported), "505 HTTP Version Not Supported");
}

//! FindHeader: 大小写不敏感查找 header，返回迭代器
TEST(common, FindHeader_ExactCase)
{
    Headers headers;
    headers["Content-Type"] = "text/plain";
    headers["Content-Length"] = "100";

    auto it = FindHeader(headers, "Content-Type");
    ASSERT_NE(it, headers.end());
    EXPECT_EQ(it->second, "text/plain");

    it = FindHeader(headers, "Content-Length");
    ASSERT_NE(it, headers.end());
    EXPECT_EQ(it->second, "100");
}

TEST(common, FindHeader_LowerCase)
{
    Headers headers;
    headers["Content-Type"] = "text/plain";
    headers["Content-Length"] = "100";

    auto it = FindHeader(headers, "content-type");
    ASSERT_NE(it, headers.end());
    EXPECT_EQ(it->second, "text/plain");

    it = FindHeader(headers, "content-length");
    ASSERT_NE(it, headers.end());
    EXPECT_EQ(it->second, "100");
}

TEST(common, FindHeader_UpperCase)
{
    Headers headers;
    headers["Content-Type"] = "text/plain";

    auto it = FindHeader(headers, "CONTENT-TYPE");
    ASSERT_NE(it, headers.end());
    EXPECT_EQ(it->second, "text/plain");
}

TEST(common, FindHeader_MixedCase)
{
    Headers headers;
    headers["Content-Type"] = "text/plain";

    auto it = FindHeader(headers, "cOnTeNt-TyPe");
    ASSERT_NE(it, headers.end());
    EXPECT_EQ(it->second, "text/plain");
}

TEST(common, FindHeader_NotFound)
{
    Headers headers;
    headers["Content-Type"] = "text/plain";

    auto it = FindHeader(headers, "Authorization");
    EXPECT_EQ(it, headers.end());
}

TEST(common, FindHeader_EmptyHeaders)
{
    Headers headers;
    auto it = FindHeader(headers, "Content-Type");
    EXPECT_EQ(it, headers.end());
}

TEST(common, FindHeader_KeyStoredAsLowerCase)
{
    //! headers 中存储的 key 是小写，用标准大小写查找
    Headers headers;
    headers["content-type"] = "text/plain";

    auto it = FindHeader(headers, "Content-Type");
    ASSERT_NE(it, headers.end());
    EXPECT_EQ(it->second, "text/plain");
}

//! GetHeader: 大小写不敏感查找 header，返回值
TEST(common, GetHeader_Found)
{
    Headers headers;
    headers["Content-Type"] = "text/plain";

    EXPECT_EQ(GetHeader(headers, "Content-Type"), "text/plain");
    EXPECT_EQ(GetHeader(headers, "content-type"), "text/plain");
    EXPECT_EQ(GetHeader(headers, "CONTENT-TYPE"), "text/plain");
}

TEST(common, GetHeader_NotFound)
{
    Headers headers;
    headers["Content-Type"] = "text/plain";

    EXPECT_EQ(GetHeader(headers, "Authorization"), "");
}

TEST(common, GetHeader_EmptyHeaders)
{
    Headers headers;
    EXPECT_EQ(GetHeader(headers, "Content-Type"), "");
}

}
}
}
