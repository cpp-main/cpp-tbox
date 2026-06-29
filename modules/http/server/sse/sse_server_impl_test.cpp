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
 * Copyright (c) 2026 Hevake and contributors, all rights reserved.
 *
 * This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
 * Use of this source code is governed by MIT license that can be found
 * in the LICENSE file in the root of the source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */
#include <gtest/gtest.h>
#include <tbox/http/request.h>
#include "sse_server_impl.h"

namespace tbox {
namespace http {
namespace sse {

TEST(SseServerImpl, DetectSseRequest)
{
    //! 合法的 SSE 请求：GET + Accept: text/event-stream
    http::Request req;
    req.method = http::Method::kGet;
    req.http_ver = http::HttpVer::k1_1;
    req.headers["Accept"] = "text/event-stream";

    EXPECT_TRUE(SseServer::Impl::IsSseRequest(req));
}

TEST(SseServerImpl, DetectSseRequestWithMultipleAccept)
{
    //! Accept 头包含多个值时，仍能检测到 text/event-stream
    http::Request req;
    req.method = http::Method::kGet;
    req.headers["Accept"] = "text/event-stream, text/html;q=0.9";

    EXPECT_TRUE(SseServer::Impl::IsSseRequest(req));
}

TEST(SseServerImpl, DetectNonSseRequestNoAccept)
{
    //! 缺少 Accept 头：不是 SSE 请求
    http::Request req;
    req.method = http::Method::kGet;

    EXPECT_FALSE(SseServer::Impl::IsSseRequest(req));
}

TEST(SseServerImpl, DetectNonSseRequestHtmlAccept)
{
    //! Accept 头不包含 text/event-stream：不是 SSE 请求
    http::Request req;
    req.method = http::Method::kGet;
    req.headers["Accept"] = "text/html";

    EXPECT_FALSE(SseServer::Impl::IsSseRequest(req));
}

TEST(SseServerImpl, DetectNonSsePostRequest)
{
    //! POST 方法：不是 SSE 请求（SSE 必须是 GET）
    http::Request req;
    req.method = http::Method::kPost;
    req.headers["Accept"] = "text/event-stream";

    EXPECT_FALSE(SseServer::Impl::IsSseRequest(req));
}

TEST(SseServerImpl, DetectNonSsePutRequest)
{
    //! PUT 方法：不是 SSE 请求
    http::Request req;
    req.method = http::Method::kPut;
    req.headers["Accept"] = "text/event-stream";

    EXPECT_FALSE(SseServer::Impl::IsSseRequest(req));
}

}
}
}
