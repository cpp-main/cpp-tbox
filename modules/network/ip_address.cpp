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

#include <arpa/inet.h>
#include <sstream>

namespace tbox {
namespace network {

std::string IPAddress::FormatInvalid::Format(const std::string &str)
{
    std::ostringstream oss;
    oss << "'" << str << "' is not IPv4 string";
    return oss.str();
}

std::string IPAddress::toString() const
{
    char ip_str_buff[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &ip_, ip_str_buff, INET_ADDRSTRLEN);
    return ip_str_buff;
}

IPAddress IPAddress::FromString(const std::string &ip_str) {
    struct in_addr addr;
    auto ret = inet_aton(ip_str.c_str(), &addr);
    if (ret == 0)
        throw FormatInvalid(ip_str);
    return IPAddress(addr.s_addr);
}

IPAddress IPAddress::Any()
{
    return IPAddress(0);
}

IPAddress IPAddress::Loop()
{
    return IPAddress(0x0100007F);
}

}
}
