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
#include <cstring>
#include "request_parser.h"

namespace tbox {
namespace http {
namespace server {
namespace {

TEST(RequestParser, Get)
{
    const char *text = \
        "GET /index.html HTTP/1.1\r\n"
        "Content-Type: plain/text\r\n"
        "Content-Length: 0\r\n"
        "\r\n"
        ;
    size_t text_len = ::strlen(text);
    RequestParser pp;
    EXPECT_EQ(pp.parse(text, text_len), text_len);
    ASSERT_EQ(pp.state(), RequestParser::State::kFinishedAll);
    auto req = pp.getRequest();
    ASSERT_NE(req, nullptr);
    EXPECT_EQ(req->method, Method::kGet);
    EXPECT_EQ(req->url.path, "/index.html");
    EXPECT_EQ(req->http_ver, HttpVer::k1_1);
    EXPECT_EQ(req->headers["Content-Type"], "plain/text");
    EXPECT_EQ(req->headers["Content-Length"], "0");
    EXPECT_EQ(req->body, "");
    delete req;
}

TEST(RequestParser, Get_1)
{
    const char *text = \
        "GET /?sl=auto&tl=en&text=%E5%86%92%E5%8F%B7&op=translate HTTP/1.1\r\n"
        "Host: 192.168.0.15:55555\r\n"
        "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:99.0) Gecko/20100101 Firefox/99.0\r\n"
        "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,*/*;q=0.8\r\n"
        "Accept-Language: zh-CN,zh;q=0.8,zh-TW;q=0.7,zh-HK;q=0.5,en-US;q=0.3,en;q=0.2\r\n"
        "Token_1: \r\n"
        "Token_2:\r\n"
        "Accept-Encoding: gzip, deflate\r\n"
        "DNT: 1\r\n"
        "Connection: keep-alive\r\n"
        "Upgrade-Insecure-Requests: 1\r\n"
        "\r\n"
        ;
    size_t text_len = ::strlen(text);
    RequestParser pp;
    EXPECT_EQ(pp.parse(text, text_len), text_len);
    ASSERT_EQ(pp.state(), RequestParser::State::kFinishedAll);
    auto req = pp.getRequest();
    ASSERT_NE(req, nullptr);
    EXPECT_EQ(req->method, Method::kGet);
    EXPECT_EQ(req->url.path, "/");
    ASSERT_EQ(req->url.query.size(), 4u);
    EXPECT_EQ(req->url.query["sl"], "auto");
    EXPECT_EQ(req->url.query["tl"], "en");
    EXPECT_EQ(req->url.query["text"], "\xE5\x86\x92\xE5\x8F\xB7");
    EXPECT_EQ(req->url.query["op"], "translate");
    EXPECT_EQ(req->http_ver, HttpVer::k1_1);
    EXPECT_EQ(req->headers["Host"], "192.168.0.15:55555");
    EXPECT_EQ(req->headers["User-Agent"], "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:99.0) Gecko/20100101 Firefox/99.0");
    EXPECT_EQ(req->headers["Accept"], "text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,*/*;q=0.8");
    EXPECT_EQ(req->headers["Accept-Language"], "zh-CN,zh;q=0.8,zh-TW;q=0.7,zh-HK;q=0.5,en-US;q=0.3,en;q=0.2");
    EXPECT_EQ(req->headers["Accept-Encoding"], "gzip, deflate");
    EXPECT_EQ(req->headers["DNT"], "1");
    EXPECT_EQ(req->headers["Connection"], "keep-alive");
    EXPECT_EQ(req->headers["Upgrade-Insecure-Requests"], "1");
    EXPECT_EQ(req->headers["Token_1"], "");
    EXPECT_EQ(req->headers["Token_2"], "");
    EXPECT_EQ(req->body, "");
    delete req;
}

//! 测试 POST 请求，即含用 body 的请求
TEST(RequestParser, Post)
{
    const char *text = \
        "POST /login.php HTTP/1.1\r\n"
        "Content-Type: plain/text\r\n"
        "Content-Length: 26\r\n"
        "\r\n"
        "username=hevake&pwd=abc123"
        ;
    size_t text_len = ::strlen(text);
    RequestParser pp;
    EXPECT_EQ(pp.parse(text, text_len), text_len);
    ASSERT_EQ(pp.state(), RequestParser::State::kFinishedAll);
    auto req = pp.getRequest();
    ASSERT_NE(req, nullptr);
    EXPECT_EQ(req->method, Method::kPost);
    EXPECT_EQ(req->url.path, "/login.php");
    EXPECT_EQ(req->http_ver, HttpVer::k1_1);
    EXPECT_EQ(req->headers["Content-Type"], "plain/text");
    EXPECT_EQ(req->headers["Content-Length"], "26");
    EXPECT_EQ(req->body, "username=hevake&pwd=abc123");
    delete req;
}

//! 测试 POST 请求，即含用 body 的请求
TEST(RequestParser, Post_NoContentLength)
{
    const char *text = \
        "POST /login.php HTTP/1.1\r\n"
        "Content-Type: plain/text\r\n"
        "\r\n"
        "username=hevake&pwd=abc123"
        ;
    size_t text_len = ::strlen(text);
    RequestParser pp;
    EXPECT_EQ(pp.parse(text, text_len), text_len);
    ASSERT_EQ(pp.state(), RequestParser::State::kFinishedAll);
    auto req = pp.getRequest();
    ASSERT_NE(req, nullptr);
    EXPECT_EQ(req->method, Method::kPost);
    EXPECT_EQ(req->url.path, "/login.php");
    EXPECT_EQ(req->http_ver, HttpVer::k1_1);
    EXPECT_EQ(req->headers["Content-Type"], "plain/text");
    EXPECT_EQ(req->body, "username=hevake&pwd=abc123");
    delete req;
}

//! 测试同一个连接下多次请求
TEST(RequestParser, GetAndPost)
{
    const char *text = \
        "GET /index.html HTTP/1.1\r\n"
        "Content-Type: plain/text\r\n"
        "Content-Length: 0\r\n"
        "\r\n"
        "POST /login.php HTTP/1.1\r\n"
        "Content-Type: plain/text\r\n"
        "Content-Length: 26\r\n"
        "\r\n"
        "username=hevake&pwd=abc123"
        ;
    size_t text_len = ::strlen(text);
    RequestParser pp;
    size_t pos = pp.parse(text, text_len);
    ASSERT_EQ(pp.state(), RequestParser::State::kFinishedAll);
    auto req1 = pp.getRequest();
    ASSERT_NE(req1, nullptr);
    EXPECT_EQ(req1->method, Method::kGet);
    EXPECT_EQ(req1->url.path, "/index.html");
    EXPECT_EQ(req1->http_ver, HttpVer::k1_1);
    EXPECT_EQ(req1->headers["Content-Type"], "plain/text");
    EXPECT_EQ(req1->headers["Content-Length"], "0");
    EXPECT_EQ(req1->body, "");
    delete req1;

    ASSERT_EQ(pp.parse(text + pos, text_len - pos), text_len - pos);
    ASSERT_EQ(pp.state(), RequestParser::State::kFinishedAll);
    auto req2 = pp.getRequest();
    ASSERT_NE(req2, nullptr);
    EXPECT_EQ(req2->method, Method::kPost);
    EXPECT_EQ(req2->url.path, "/login.php");
    EXPECT_EQ(req2->http_ver, HttpVer::k1_1);
    EXPECT_EQ(req2->headers["Content-Type"], "plain/text");
    EXPECT_EQ(req2->headers["Content-Length"], "26");
    EXPECT_EQ(req2->body, "username=hevake&pwd=abc123");
    delete req2;
}

//! 测试一个请求分多次发送的情况
TEST(RequestParser, PostIn3Pice)
{
    const char *text1 = \
        "POST /login.php HTTP/1.1\r\n"
        "Content-Type: plain/text\r\n"
        ;

    size_t text1_len = ::strlen(text1);
    RequestParser pp;
    EXPECT_EQ(pp.parse(text1, text1_len), text1_len);
    EXPECT_EQ(pp.state(), RequestParser::State::kFinishedStartLine);
    auto req = pp.getRequest();
    EXPECT_EQ(req, nullptr);

    const char *text2 = \
        "Content-Length: 26\r\n"
        "\r\n"
        ;

    size_t text2_len = ::strlen(text2);
    EXPECT_EQ(pp.parse(text2, text2_len), text2_len);
    EXPECT_EQ(pp.state(), RequestParser::State::kFinishedHeads);

    const char *text3 = \
        "username=hevake&pwd=abc123";

    size_t text3_len = ::strlen(text3);
    EXPECT_EQ(pp.parse(text3, text3_len), text3_len);
    EXPECT_EQ(pp.state(), RequestParser::State::kFinishedAll);

    req = pp.getRequest();
    ASSERT_NE(req, nullptr);
    EXPECT_EQ(req->method, Method::kPost);
    EXPECT_EQ(req->url.path, "/login.php");
    EXPECT_EQ(req->http_ver, HttpVer::k1_1);
    EXPECT_EQ(req->headers["Content-Type"], "plain/text");
    EXPECT_EQ(req->headers["Content-Length"], "26");
    EXPECT_EQ(req->body, "username=hevake&pwd=abc123");
    delete req;
}

//! 测试起始行不完整的情况
TEST(RequestParser, StartLineNotEnough)
{
    RequestParser pp;

    std::string text = "POST /login.php ";
    EXPECT_EQ(pp.parse(text.c_str(), text.size()), 0);
    EXPECT_EQ(pp.state(), RequestParser::State::kInit);

    text += "HTTP/1.1\r\n"
        "Content-Type: plain/text\r\n"
        "Content-Length: 26\r\n"
        "\r\n"
        "username=hevake&pwd=abc123";
    EXPECT_EQ(pp.parse(text.c_str(), text.size()), text.size());
    EXPECT_EQ(pp.state(), RequestParser::State::kFinishedAll);

    auto req = pp.getRequest();
    ASSERT_NE(req, nullptr);
    EXPECT_EQ(req->method, Method::kPost);
    EXPECT_EQ(req->url.path, "/login.php");
    EXPECT_EQ(req->http_ver, HttpVer::k1_1);
    EXPECT_EQ(req->headers["Content-Type"], "plain/text");
    EXPECT_EQ(req->headers["Content-Length"], "26");
    EXPECT_EQ(req->body, "username=hevake&pwd=abc123");
    delete req;

}

//! 测试Head不完整的情况
TEST(RequestParser, HeaderNotEnough)
{
    RequestParser pp;

    std::string text = \
        "POST /login.php HTTP/1.1\r\n"
        "Content-Type: plain";
    EXPECT_EQ(pp.parse(text.c_str(), text.size()), 26);
    EXPECT_EQ(pp.state(), RequestParser::State::kFinishedStartLine);

    text.erase(0, 26);
    text += "/text\r\n"
        "Content-Length: 26\r\n"
        "\r\n"
        "username=hevake&pwd=abc123";
    EXPECT_EQ(pp.parse(text.c_str(), text.size()), text.size());
    EXPECT_EQ(pp.state(), RequestParser::State::kFinishedAll);

    auto req = pp.getRequest();
    ASSERT_NE(req, nullptr);
    EXPECT_EQ(req->method, Method::kPost);
    EXPECT_EQ(req->url.path, "/login.php");
    EXPECT_EQ(req->http_ver, HttpVer::k1_1);
    EXPECT_EQ(req->headers["Content-Type"], "plain/text");
    EXPECT_EQ(req->headers["Content-Length"], "26");
    EXPECT_EQ(req->body, "username=hevake&pwd=abc123");
    delete req;
}

//! 测试Body不完整的情况
TEST(RequestParser, BodyNotEnough)
{
    RequestParser pp;

    std::string text = \
        "POST /login.php HTTP/1.1\r\n"
        "Content-Type: plain/text\r\n"
        "Content-Length: 26\r\n"
        "\r\n"
        "username=hevake&";

    auto pos = pp.parse(text.c_str(), text.size());
    EXPECT_EQ(pp.state(), RequestParser::State::kFinishedHeads);

    text.erase(0, pos);
    text += "pwd=abc123";
    EXPECT_EQ(pp.parse(text.c_str(), text.size()), text.size());
    EXPECT_EQ(pp.state(), RequestParser::State::kFinishedAll);

    auto req = pp.getRequest();
    ASSERT_NE(req, nullptr);
    EXPECT_EQ(req->method, Method::kPost);
    EXPECT_EQ(req->url.path, "/login.php");
    EXPECT_EQ(req->http_ver, HttpVer::k1_1);
    EXPECT_EQ(req->headers["Content-Type"], "plain/text");
    EXPECT_EQ(req->headers["Content-Length"], "26");
    EXPECT_EQ(req->body, "username=hevake&pwd=abc123");
    delete req;
}

//! 测试StartLine格式错误的情况
TEST(RequestParser, StartLineError_NoMethod)
{
    RequestParser pp;

    std::string text = 
        "/login.php HTTP/1.1\r\n"
        ;
    pp.parse(text.c_str(), text.size());
    EXPECT_EQ(pp.state(), RequestParser::State::kFail);
}

TEST(RequestParser, StartLineError_MethodNotSupport)
{
    RequestParser pp;

    std::string text = 
        "XXX /login.php HTTP/1.1\r\n"
        ;
    pp.parse(text.c_str(), text.size());
    EXPECT_EQ(pp.state(), RequestParser::State::kFail);
}

TEST(RequestParser, StartLineError_NoUrl)
{
    RequestParser pp;

    std::string text = 
        "POST HTTP/1.1\r\n"
        ;
    pp.parse(text.c_str(), text.size());
    EXPECT_EQ(pp.state(), RequestParser::State::kFail);
}

TEST(RequestParser, StartLineError_NoVer)
{
    RequestParser pp;

    std::string text = 
        "POST /login.php\r\n"
        ;
    pp.parse(text.c_str(), text.size());
    EXPECT_EQ(pp.state(), RequestParser::State::kFail);
}

TEST(RequestParser, StartLineError_VerUnknow)
{
    RequestParser pp;

    std::string text = 
        "POST /login.php XXXX\r\n"
        ;
    pp.parse(text.c_str(), text.size());
    EXPECT_EQ(pp.state(), RequestParser::State::kFail);
}

TEST(RequestParser, HeaderError_NoColon)
{
    RequestParser pp;

    std::string text = \
        "POST /login.php HTTP/1.1\r\n"
        "Content-Type plain/text\r\n"
        ;
    pp.parse(text.c_str(), text.size());
    EXPECT_EQ(pp.state(), RequestParser::State::kFail);
}

TEST(RequestParser, HeaderError_Empty)
{
    RequestParser pp;

    std::string text = \
        "POST /login.php HTTP/1.1\r\n"
        " \r\n"
        ;
    pp.parse(text.c_str(), text.size());
    EXPECT_EQ(pp.state(), RequestParser::State::kFail);
}

}
}
}
}

