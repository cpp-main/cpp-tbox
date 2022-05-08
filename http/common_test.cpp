#include <gtest/gtest.h>
#include "common.h"
#include <sstream>

namespace tbox {
namespace http {
namespace {

using namespace std;

TEST(common, HttpVerToStream)
{
    {
        ostringstream oss;
        oss << HttpVer::k1_0;
        EXPECT_EQ(oss.str(), "HTTP/1.0");
    }
    {
        ostringstream oss;
        oss << HttpVer::k1_1;
        EXPECT_EQ(oss.str(), "HTTP/1.1");
    }
    {
        ostringstream oss;
        oss << HttpVer::k2_0;
        EXPECT_EQ(oss.str(), "HTTP/2.0");
    }
}

TEST(common, MethodToStream)
{
    {
        ostringstream oss;
        oss << Method::kGet;
        EXPECT_EQ(oss.str(), "GET");
    }
    {
        ostringstream oss;
        oss << Method::kHead;
        EXPECT_EQ(oss.str(), "HEAD");
    }
    {
        ostringstream oss;
        oss << Method::kPut;
        EXPECT_EQ(oss.str(), "PUT");
    }
    {
        ostringstream oss;
        oss << Method::kPost;
        EXPECT_EQ(oss.str(), "POST");
    }
    {
        ostringstream oss;
        oss << Method::kDelete;
        EXPECT_EQ(oss.str(), "DELETE");
    }
}

TEST(common, StateCodeToStream)
{
    {
        ostringstream oss;
        oss << StatusCode::k200_OK;
        EXPECT_EQ(oss.str(), "200 OK");
    }
    {
        ostringstream oss;
        oss << StatusCode::k401_Unauthorized;
        EXPECT_EQ(oss.str(), "401 Unauthorized");
    }
    {
        ostringstream oss;
        oss << StatusCode::k404_NotFound;
        EXPECT_EQ(oss.str(), "404 Not Found");
    }
    {
        ostringstream oss;
        oss << StatusCode::k505_HTTPVersionNotSupported;
        EXPECT_EQ(oss.str(), "505 HTTP Version Not Supported");
    }
}

TEST(common, HttpToLocal)
{
    EXPECT_EQ(HttpToLocal("hello%20world"), "hello world");
    EXPECT_EQ(HttpToLocal(R"(%23%25%26%2b%2f%5c%3d%3f%20%2e%3a)"), R"(#%&+/\=? .:)");
    EXPECT_THROW(HttpToLocal("hello%2world"), std::out_of_range);
}

TEST(common, LocalToHttp)
{
    EXPECT_EQ(LocalToHttp("hello world"), "hello%20world");
    EXPECT_EQ(LocalToHttp(R"(#%&+/\=? .:)"), R"(%23%25%26%2b%2f%5c%3d%3f%20%2e%3a)");
}

}
}
}
