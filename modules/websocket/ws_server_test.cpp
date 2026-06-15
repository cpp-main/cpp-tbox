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
 * Copyright (c) 2025 Hevake and contributors, all rights reserved.
 *
 * This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
 * Use of this source code is governed by MIT license that can be found
 * in the LICENSE file in the root of the source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */
#include <gtest/gtest.h>

#include <tbox/http/request.h>
#include "ws_server.h"

namespace tbox {
namespace websocket {
namespace {

//! RFC 6455 Section 4.2.2 示例：
//! Sec-WebSocket-Key = "dGhlIHNhbXBsZSBub25jZQ=="
//! Sec-WebSocket-Accept = "s3pPLMBiTxaQ9kYGzzhZRbK+xOo="
const char *kTestKey = "dGhlIHNhbXBsZSBub25jZQ==";
const char *kTestAccept = "s3pPLMBiTxaQ9kYGzzhZRbK+xOo=";

}

TEST(WsHandshake, ComputeAcceptKey)
{
    //! RFC 6455 示例
    std::string accept = WsServer::ComputeWsAcceptKey(kTestKey);
    EXPECT_EQ(kTestAccept, accept);
}

TEST(WsServer, DetectUpgradeRequest)
{
    http::Request req;
    req.method = http::Method::kGet;
    req.http_ver = http::HttpVer::k1_1;
    req.headers["Upgrade"] = "websocket";
    req.headers["Connection"] = "Upgrade";
    req.headers["Sec-WebSocket-Key"] = "dGhlIHNhbXBsZSBub25jZQ==";
    req.headers["Sec-WebSocket-Version"] = "13";

    EXPECT_TRUE(WsServer::IsWsUpgradeRequest(req));
}

TEST(WsServer, DetectNonUpgradeRequest)
{
    http::Request req;
    req.method = http::Method::kGet;
    req.http_ver = http::HttpVer::k1_1;

    EXPECT_FALSE(WsServer::IsWsUpgradeRequest(req));
}

TEST(WsServer, DetectPostNotUpgrade)
{
    http::Request req;
    req.method = http::Method::kPost;
    req.headers["Upgrade"] = "websocket";

    EXPECT_FALSE(WsServer::IsWsUpgradeRequest(req));
}

TEST(WsServer, DetectMissingKey)
{
    http::Request req;
    req.method = http::Method::kGet;
    req.headers["Upgrade"] = "websocket";
    req.headers["Connection"] = "Upgrade";
    //! 缺少 Sec-WebSocket-Key

    EXPECT_FALSE(WsServer::IsWsUpgradeRequest(req));
}

TEST(WsServer, DetectWrongVersion)
{
    http::Request req;
    req.method = http::Method::kGet;
    req.headers["Upgrade"] = "websocket";
    req.headers["Connection"] = "Upgrade";
    req.headers["Sec-WebSocket-Key"] = "dGhlIHNhbXBsZSBub25jZQ==";
    req.headers["Sec-WebSocket-Version"] = "8";  //! 不是 13

    EXPECT_FALSE(WsServer::IsWsUpgradeRequest(req));
}

}
}
