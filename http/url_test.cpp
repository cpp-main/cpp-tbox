#include "url.h"
#include <gtest/gtest.h>

namespace tbox {
namespace http {

TEST(Url, UrlToLocal)
{
    EXPECT_EQ(UrlToLocal("hello%20world"), "hello world");
    EXPECT_EQ(UrlToLocal(R"(%23%25%26%2b%2f%5c%3d%3f%20%2e%3a)"), R"(#%&+/\=? .:)");
    EXPECT_THROW(UrlToLocal("hello%2world"), std::out_of_range);
}

TEST(Url, LocalToUrl)
{
    EXPECT_EQ(LocalToUrl("hello world"), "hello%20world");
    EXPECT_EQ(LocalToUrl(R"(#%&+/\=? .:)"), R"(%23%25%26%2b%2f%5c%3d%3f%20%2e%3a)");
}

TEST(Url, UrlToString_Full)
{
    Url url;
    url.scheme = "http";
    url.host.user = "john";
    url.host.password = "123";
    url.host.host = "github.com";
    url.host.port = 80;
    url.path.path = "/index.html";
    url.path.params["a"] = "1";
    url.path.params["b"] = "2";
    url.path.query["x"] = "a";
    url.path.query["y"] = "b";
    url.path.frag = "tag1";

    EXPECT_EQ(UrlToString(url), "http://john:123@github.com:80/index.html;a=1;b=2?x=a&y=b#tag1");
}

TEST(Url, UrlToString_Short)
{
    Url url;
    url.host.host = "github.com";
    url.path.path = "/index.html";

    EXPECT_EQ(UrlToString(url), "github.com/index.html");
}

}
}
