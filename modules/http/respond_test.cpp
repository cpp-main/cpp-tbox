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
#include "respond.h"

namespace tbox {
namespace http {
namespace {

using namespace std;

TEST(Respond, ToString)
{
    Respond rsp;
    rsp.status_code = StatusCode::k200_OK;
    rsp.http_ver = HttpVer::k1_1;
    rsp.body = "hello world!";
    rsp.headers["Content-Type"] = "plain/text";

    EXPECT_TRUE(rsp.isValid());

    const char *target_str = \
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: plain/text\r\n"
        "Content-Length: 12\r\n"
        "\r\n"
        "hello world!";

    EXPECT_EQ(rsp.toString(), target_str);
}

}
}
}
