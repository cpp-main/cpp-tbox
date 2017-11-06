#include <gtest/gtest.h>
#include <sys/un.h>
#include <arpa/inet.h>

#define protected public
#define private public

#include "sockaddr.h"

using namespace tbox::network;

TEST(network_SockAddr, FromString_IPv4)
{
    SockAddr a1 = SockAddr::FromString("192.168.1.1:1000");
    EXPECT_EQ(a1.type(), SockAddr::Type::kIPv4);
    EXPECT_EQ(a1.addr_.ss_family, AF_INET);
    struct sockaddr_in *p_in = (struct sockaddr_in*)&a1.addr_;
    EXPECT_EQ(p_in->sin_addr.s_addr, 0x0101a8c0u);
    EXPECT_EQ(p_in->sin_port, htons(1000));
}

TEST(network_SockAddr, FromString_Local)
{
    SockAddr a1 = SockAddr::FromString("/tmp/test.sock");
    EXPECT_EQ(a1.type(), SockAddr::Type::kLocal);
    EXPECT_EQ(a1.addr_.ss_family, AF_LOCAL);
    struct sockaddr_un *p_un = (struct sockaddr_un*)&a1.addr_;
    EXPECT_EQ(p_un->sun_path, "/tmp/test.sock");
}
