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
#include "url.h"
#include <gtest/gtest.h>

namespace tbox {
namespace http {

TEST(Url, UrlDecode)
{
    EXPECT_EQ(UrlDecode("hello%20world"), "hello world");
    EXPECT_EQ(UrlDecode(R"(%23%25%26%2B%2F%5C%3D%3F%20%2E%3A)"), R"(#%&+/\=? .:)");
    EXPECT_THROW(UrlDecode("hello%2world"), std::runtime_error);
    EXPECT_THROW(UrlDecode("hello%"), std::runtime_error);
}

TEST(Url, UrlEncode)
{
    EXPECT_EQ(UrlEncode("hello world"), "hello%20world");
    EXPECT_EQ(UrlEncode(R"(#%&+/\=? .:)"), R"(%23%25%26%2B%2F%5C%3D%3F%20%2E%3A)");
    EXPECT_EQ(UrlEncode("\xA0\xB0"), R"(%A0%B0)");
}

TEST(Url, StringToUrlHost_Full)
{
    Url::Host host;
    ASSERT_TRUE(StringToUrlHost("user:password@github.com:80", host));
    EXPECT_EQ(host.user, "user");
    EXPECT_EQ(host.password, "password");
    EXPECT_EQ(host.host, "github.com");
    EXPECT_EQ(host.port, 80);
}

TEST(Url, StringToUrlHost_Short)
{
    Url::Host host;
    ASSERT_TRUE(StringToUrlHost("github.com", host));
    EXPECT_EQ(host.user, "");
    EXPECT_EQ(host.password, "");
    EXPECT_EQ(host.host, "github.com");
    EXPECT_EQ(host.port, 0);
}

TEST(Url, StringToUrlHost_UserNoPassword)
{
    Url::Host host;
    ASSERT_TRUE(StringToUrlHost("user@github.com", host));
    EXPECT_EQ(host.user, "user");
    EXPECT_EQ(host.password, "");
    EXPECT_EQ(host.host, "github.com");
    EXPECT_EQ(host.port, 0);
}

TEST(Url, StringToUrlHost_Error)
{
    Url::Host host;
    EXPECT_FALSE(StringToUrlHost("github.com:abc123", host));
}

TEST(Url, StringToUrlPath_Full)
{
    Url::Path path;
    ASSERT_TRUE(StringToUrlPath("/path/index.html;a=1;b=2?x=a&y=b#frag", path));
    EXPECT_EQ(path.path, "/path/index.html");
    EXPECT_EQ(path.frag, "frag");
    ASSERT_EQ(path.params.size(), 2u);
    EXPECT_EQ(path.params["a"], "1");
    EXPECT_EQ(path.params["b"], "2");
    ASSERT_EQ(path.query.size(), 2u);
    EXPECT_EQ(path.query["x"], "a");
    EXPECT_EQ(path.query["y"], "b");
}

TEST(Url, StringToUrlPath_Short)
{
    Url::Path path;
    ASSERT_TRUE(StringToUrlPath("/", path));
    EXPECT_EQ(path.path, "/");
    EXPECT_EQ(path.frag, "");
    ASSERT_EQ(path.params.size(), 0u);
    ASSERT_EQ(path.query.size(), 0u);
}

TEST(Url, StringToUrlPath_QueryOnly)
{
    Url::Path path;
    ASSERT_TRUE(StringToUrlPath("/path/index.html?x=a&y=b", path));
    EXPECT_EQ(path.path, "/path/index.html");
    EXPECT_EQ(path.frag, "");
    ASSERT_EQ(path.params.size(), 0u);
    ASSERT_EQ(path.query.size(), 2u);
    EXPECT_EQ(path.query["x"], "a");
    EXPECT_EQ(path.query["y"], "b");
}

TEST(Url, StringToUrlPath_ParamOnly)
{
    Url::Path path;
    ASSERT_TRUE(StringToUrlPath("/path/index.html;a=1;b=2", path));
    EXPECT_EQ(path.path, "/path/index.html");
    EXPECT_EQ(path.frag, "");
    ASSERT_EQ(path.params.size(), 2u);
    EXPECT_EQ(path.params["a"], "1");
    EXPECT_EQ(path.params["b"], "2");
    ASSERT_EQ(path.query.size(), 0u);
}

TEST(Url, StringToUrlPath_FragOnly)
{
    Url::Path path;
    ASSERT_TRUE(StringToUrlPath("/index.html#frag", path));
    EXPECT_EQ(path.path, "/index.html");
    EXPECT_EQ(path.frag, "frag");
    ASSERT_EQ(path.params.size(), 0u);
    ASSERT_EQ(path.query.size(), 0u);
}

TEST(Url, StringToUrlPath_Error_Empty)
{
    Url::Path path;
    ASSERT_FALSE(StringToUrlPath("", path));
}

TEST(Url, StringToUrlPath_Error_ParamInvalid)
{
    Url::Path path;
    EXPECT_FALSE(StringToUrlPath("/;a", path));
    EXPECT_FALSE(StringToUrlPath("/;;", path));
    EXPECT_FALSE(StringToUrlPath("/;?", path));
    EXPECT_FALSE(StringToUrlPath("/;#", path));
    EXPECT_FALSE(StringToUrlPath("/;=1", path));
    EXPECT_FALSE(StringToUrlPath("/;==", path));
}

TEST(Url, UrlToString_Full)
{
    Url url;
    url.scheme = "http";
    url.host.user = "john";
    url.host.password = "123";
    url.host.host = "github.com";
    url.host.port = 80;
    url.path.path = "/index.html";
    url.path.params["a"] = "1";
    url.path.params["b"] = "2";
    url.path.query["x"] = "a";
    url.path.query["y"] = "b";
    url.path.frag = "tag1";

    EXPECT_EQ(UrlToString(url), "http://john:123@github.com:80/index.html;a=1;b=2?x=a&y=b#tag1");
}

TEST(Url, UrlHostToString_Short)
{
    Url url;
    url.host.host = "github.com";
    url.path.path = "/index.html";

    EXPECT_EQ(UrlToString(url), "github.com/index.html");
}

TEST(Url, StringToUrl_Full)
{
    Url url;
    ASSERT_TRUE(StringToUrl("http://john:123@github.com:80/index.html;a=1;b=2?x=a&y=b#tag1", url));

    EXPECT_EQ(url.scheme, "http");
    EXPECT_EQ(url.host.user, "john");
    EXPECT_EQ(url.host.password, "123");
    EXPECT_EQ(url.host.host, "github.com");
    EXPECT_EQ(url.host.port, 80);
    EXPECT_EQ(url.path.path, "/index.html");
    ASSERT_EQ(url.path.params.size(), 2u);
    EXPECT_EQ(url.path.params["a"], "1");
    EXPECT_EQ(url.path.params["b"], "2");
    ASSERT_EQ(url.path.query.size(), 2u);
    EXPECT_EQ(url.path.query["x"], "a");
    EXPECT_EQ(url.path.query["y"], "b");
    EXPECT_EQ(url.path.frag, "tag1");
}

TEST(Url, StringToUrl_Short)
{
    Url url;
    ASSERT_TRUE(StringToUrl("github.com", url));

    EXPECT_EQ(url.scheme, "");
    EXPECT_EQ(url.host.user, "");
    EXPECT_EQ(url.host.password, "");
    EXPECT_EQ(url.host.host, "github.com");
    EXPECT_EQ(url.host.port, 0);
    EXPECT_EQ(url.path.path, "/");
    ASSERT_EQ(url.path.params.size(), 0u);
    ASSERT_EQ(url.path.query.size(), 0u);
    EXPECT_EQ(url.path.frag, "");
}

}
}
