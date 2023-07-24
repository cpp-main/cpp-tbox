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
#include <sys/un.h>
#include <arpa/inet.h>

#define protected public
#define private public

#include "sockaddr.h"

using namespace tbox::network;

TEST(SockAddr, FromString_IPv4)
{
    SockAddr a1 = SockAddr::FromString("192.168.1.1:1000");
    EXPECT_EQ(a1.type(), SockAddr::Type::kIPv4);
    EXPECT_EQ(a1.addr_.ss_family, AF_INET);
    struct sockaddr_in *p_in = (struct sockaddr_in*)&a1.addr_;
    EXPECT_EQ(p_in->sin_addr.s_addr, 0x0101a8c0u);
    EXPECT_EQ(p_in->sin_port, htons(1000));
}

TEST(SockAddr, FromString_Local)
{
    SockAddr a1 = SockAddr::FromString("/tmp/test.sock");
    EXPECT_EQ(a1.type(), SockAddr::Type::kLocal);
    EXPECT_EQ(a1.addr_.ss_family, AF_LOCAL);
    struct sockaddr_un *p_un = (struct sockaddr_un*)&a1.addr_;
    EXPECT_STREQ(p_un->sun_path, "/tmp/test.sock");
}

TEST(SockAddr, ToStringIPv4)
{
    SockAddr a = SockAddr::FromString("12.34.56.78:60000");
    EXPECT_EQ(a.toString(), "12.34.56.78:60000");
}

TEST(SockAddr, ToStringLocal)
{
    SockAddr a = SockAddr::FromString("/tmp/test.sock");
    EXPECT_EQ(a.toString(), "/tmp/test.sock");
}

TEST(SockAddr, get_IPv4)
{
    SockAddr a = SockAddr::FromString("12.34.56.78:60000");
    IPAddress ip;
    uint16_t port;
    EXPECT_TRUE(a.get(ip, port));
    EXPECT_EQ(ip.toString(), "12.34.56.78");
    EXPECT_EQ(port, 60000);
}

TEST(SockAddr, get_Local)
{
    SockAddr a = SockAddr::FromString("/tmp/test.sock");
    IPAddress ip;
    uint16_t port;
    EXPECT_FALSE(a.get(ip, port));
}

TEST(SockAddr, toSockAddrIPv4)
{
    SockAddr a = SockAddr::FromString("12.34.56.78:60000");
    struct sockaddr_in addr;
    a.toSockAddr(addr);

    EXPECT_EQ(addr.sin_family, AF_INET);
    EXPECT_EQ(addr.sin_addr.s_addr, 0x4e38220cu);
    EXPECT_EQ(addr.sin_port, htons(60000));

}

TEST(SockAddr, toSockAddrLocal)
{
    SockAddr a = SockAddr::FromString("/tmp/test.sock");
    struct sockaddr_un addr;
    bzero(&addr, sizeof(addr));
    a.toSockAddr(addr);

    EXPECT_EQ(addr.sun_family, AF_LOCAL);
    EXPECT_STREQ(addr.sun_path, "/tmp/test.sock");
}

TEST(SockAddr, Equal)
{
    EXPECT_EQ(SockAddr::FromString("127.0.0.1:5566"), SockAddr::FromString("127.0.0.1:5566"));
    EXPECT_NE(SockAddr::FromString("127.0.0.1:5566"), SockAddr::FromString("127.0.0.1:22"));
    EXPECT_NE(SockAddr::FromString("127.0.0.1:5566"), SockAddr::FromString("10.0.0.1:5566"));
    EXPECT_NE(SockAddr::FromString("127.0.0.1:5566"), SockAddr::FromString("/tmp/test.sock"));
}
