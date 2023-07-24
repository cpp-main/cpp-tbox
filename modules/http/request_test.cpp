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
#include "request.h"

namespace tbox {
namespace http {
namespace {

using namespace std;

TEST(Request, ToString_Get)
{
    Request req;
    req.method = Method::kGet;
    req.http_ver = HttpVer::k1_1;
    req.url.path = "/get_user_info.php";
    req.url.query["id"] = "john hans";
    req.headers["Content-Type"] = "plain/text";

    EXPECT_TRUE(req.isValid());

    const char *target_str = \
        "GET /get_user_info.php?id=john%20hans HTTP/1.1\r\n"
        "Content-Type: plain/text\r\n"
        "Content-Length: 0\r\n"
        "\r\n"
        ;

    EXPECT_EQ(req.toString(), target_str);
}

TEST(Request, ToString_Post)
{
    Request req;
    req.method = Method::kPost;
    req.http_ver = HttpVer::k1_1;
    req.url.path = "/login.php";
    req.url.frag = "tag";
    req.headers["Content-Type"] = "plain/text";
    req.body = "username=hevake&pwd=abc123";

    EXPECT_TRUE(req.isValid());

    const char *target_str = \
        "POST /login.php#tag HTTP/1.1\r\n"
        "Content-Type: plain/text\r\n"
        "Content-Length: 26\r\n"
        "\r\n"
        "username=hevake&pwd=abc123"
        ;

    EXPECT_EQ(req.toString(), target_str);
}

}
}
}
