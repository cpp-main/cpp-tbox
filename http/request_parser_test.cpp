#include <gtest/gtest.h>
#include <cstring>
#include "request_parser.h"

namespace tbox {
namespace http {
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
    EXPECT_EQ(req->method(), Method::kGet);
    EXPECT_EQ(req->url(), "/index.html");
    EXPECT_EQ(req->http_ver(), HttpVer::k1_1);
    EXPECT_EQ(req->headers["Content-Type"], "plain/text");
    EXPECT_EQ(req->headers["Content-Length"], "0");
    EXPECT_EQ(req->body(), "");
    delete req;
}

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
    EXPECT_EQ(req->method(), Method::kPost);
    EXPECT_EQ(req->url(), "/login.php");
    EXPECT_EQ(req->http_ver(), HttpVer::k1_1);
    EXPECT_EQ(req->headers["Content-Type"], "plain/text");
    EXPECT_EQ(req->headers["Content-Length"], "26");
    EXPECT_EQ(req->body(), "username=hevake&pwd=abc123");
    delete req;
}

}
}
}
