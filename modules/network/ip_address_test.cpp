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
#include "ip_address.h"
#include <gtest/gtest.h>

using namespace tbox::network;

TEST(IPAddress, And)
{
    IPAddress a = IPAddress::FromString("192.168.11.234");
    IPAddress b = IPAddress::FromString("255.255.255.0");
    IPAddress c = a & b;
    EXPECT_EQ(c.toString(), "192.168.11.0");
}

TEST(IPAddress, Or)
{
    IPAddress a = IPAddress::FromString("192.168.11.234");
    IPAddress b = IPAddress::FromString("0.0.0.255");
    IPAddress c = a | b;
    EXPECT_EQ(c.toString(), "192.168.11.255");
}

TEST(IPAddress, Invert)
{
    IPAddress a = IPAddress::FromString("0.0.0.255");
    IPAddress c = ~a;
    EXPECT_EQ(c.toString(), "255.255.255.0");
}

TEST(IPAddress, CalcBroadcastAddress)
{
    IPAddress ip = IPAddress::FromString("192.168.11.234");
    IPAddress mask = IPAddress::FromString("255.255.255.0");
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
