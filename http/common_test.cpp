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

}
}
}
