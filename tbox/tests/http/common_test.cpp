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
