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

}
}
}
