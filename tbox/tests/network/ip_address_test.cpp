#include "ip_address.h"
#include <gtest/gtest.h>

using namespace tbox::network;

TEST(IPAddress, And)
{
    IPAddress a("192.168.11.234");
    IPAddress b("255.255.255.0");
    IPAddress c = a & b;
    EXPECT_EQ(c.toString(), "192.168.11.0");
}

TEST(IPAddress, Or)
{
    IPAddress a("192.168.11.234");
    IPAddress b("0.0.0.255");
    IPAddress c = a | b;
    EXPECT_EQ(c.toString(), "192.168.11.255");
}

TEST(IPAddress, Invert)
{
    IPAddress a("0.0.0.255");
    IPAddress c = ~a;
    EXPECT_EQ(c.toString(), "255.255.255.0");
}

TEST(IPAddress, CalcBroadcastAddress)
{
    IPAddress ip("192.168.11.234");
    IPAddress mask("255.255.255.0");
    IPAddress broadcast = ip | ~mask;
    EXPECT_EQ(broadcast.toString(), "192.168.11.255");
}

TEST(IPAddress, Loop)
{
    EXPECT_EQ(IPAddress::Loop().toString(), "127.0.0.1");
}

TEST(IPAddress, Any)
{
    EXPECT_EQ(IPAddress::Any().toString(), "0.0.0.0");
}
