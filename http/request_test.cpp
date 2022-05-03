#include <gtest/gtest.h>
#include "request.h"

namespace tbox {
namespace http {
namespace {

using namespace std;

TEST(Request, ToString_Get)
{
    Request req;
    req.set_method(Method::kGet);
    req.set_http_ver(HttpVer::k1_1);
    req.set_url("/index.html");
    req.headers["Content-Type"] = "plain/text";

    EXPECT_TRUE(req.isValid());

    const char *target_str = \
        "GET /index.html HTTP/1.1\r\n"
        "Content-Type: plain/text\r\n"
        "Content-Length: 0\r\n"
        "\r\n"
        ;

    EXPECT_EQ(req.toString(), target_str);
}

TEST(Request, ToString_Post)
{
    Request req;
    req.set_method(Method::kPost);
    req.set_http_ver(HttpVer::k1_1);
    req.set_url("/login.php");
    req.headers["Content-Type"] = "plain/text";
    req.set_body("username=hevake&pwd=abc123");

    EXPECT_TRUE(req.isValid());

    const char *target_str = \
        "POST /login.php HTTP/1.1\r\n"
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
