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
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>

#include <tbox/base/log.h>
#include <sstream>

#include "sockaddr.h"

namespace tbox {
namespace network {

using namespace std;

const size_t kSockAddrUnHeadSize = (size_t)&(((struct sockaddr_un*)0)->sun_path);

SockAddr::SockAddr()
{
    bzero(&addr_, sizeof(addr_));
}

SockAddr::SockAddr(const IPAddress &ip, uint16_t port)
{
    bzero(&addr_, sizeof(addr_));

    struct sockaddr_in *p_addr = (struct sockaddr_in *)&addr_;
    p_addr->sin_family = AF_INET;
    p_addr->sin_addr.s_addr = ip;
    p_addr->sin_port = htons(port);

    len_ = sizeof(struct sockaddr_in);
}

SockAddr::SockAddr(const DomainSockPath &path)
{
    bzero(&addr_, sizeof(addr_));

    struct sockaddr_un *p_addr = (struct sockaddr_un *)&addr_;
    p_addr->sun_family = AF_LOCAL;

    auto &sock_path = path.get();
    //!NOTE: sock_path 字串中可能存在\0字符，所以不能当普通字串处理
    ::memcpy(p_addr->sun_path, sock_path.data(), sock_path.size());
    len_ = kSockAddrUnHeadSize + sock_path.size();
}

SockAddr::SockAddr(const struct sockaddr &addr, socklen_t len)
{
    ::memcpy(&addr_, &addr, len);
    len_ = len;
}

SockAddr::SockAddr(const struct sockaddr_in &addr)
{
    ::memcpy(&addr_, &addr, sizeof(addr));
    len_ = sizeof(addr);
}

SockAddr::SockAddr(const SockAddr &other)
{
    len_ = other.len_;
    ::memcpy(&addr_, &other.addr_, len_);
}

SockAddr& SockAddr::operator = (const SockAddr &other)
{
    if (this != &other) {
        len_ = other.len_;
        ::memcpy(&addr_, &other.addr_, len_);
    }
    return *this;
}

SockAddr SockAddr::FromString(const string &addr_str)
{
    //! 看看是不是 '192.168.23.44:9999' 格式的
    auto colon_pos = addr_str.find(":");
    if (colon_pos != string::npos) {
        //! 当成 IPv4 处理
        auto ipv4_str = addr_str.substr(0, colon_pos) ;
        auto port_str = addr_str.substr(colon_pos + 1) ;

        try {
            uint16_t port = stoi(port_str);
            return SockAddr(IPAddress::FromString(ipv4_str), port);
        } catch (const std::exception &e) {
            return SockAddr();
        }
    } else {
        //! 作为Local处理
        return SockAddr(DomainSockPath(addr_str));
    }
}

SockAddr::Type SockAddr::type() const
{
    switch (addr_.ss_family) {
        case AF_INET:   return Type::kIPv4;
        case AF_LOCAL:  return Type::kLocal;
        default:        return Type::kNone;
    }
}

namespace {
string ToIPv4String(const struct sockaddr_storage &addr)
{
    struct sockaddr_in *p_addr = (struct sockaddr_in*)&addr;
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &p_addr->sin_addr, ip, sizeof(ip));
    ostringstream oss;
    oss << ip << ':' << ntohs(p_addr->sin_port);
    return oss.str();
}

string ToLocalString(const struct sockaddr_storage &addr, socklen_t len)
{
    struct sockaddr_un *p_addr = (struct sockaddr_un*)&addr;
    return string(p_addr->sun_path, len - kSockAddrUnHeadSize); //! Path 未必是以 \0 结束的，要指定长度
}
}

string SockAddr::toString() const
{
    switch (addr_.ss_family) {
        case AF_INET:   return ToIPv4String(addr_);
        case AF_LOCAL:  return ToLocalString(addr_, len_);
    }
    LogWarn("unspport family");
    return "";
}

bool SockAddr::get(IPAddress &ip, uint16_t &port) const
{
    if (addr_.ss_family == AF_INET) {
        struct sockaddr_in *p_addr = (struct sockaddr_in*)&addr_;
        ip = p_addr->sin_addr.s_addr;
        port = ntohs(p_addr->sin_port);
        return true;
    }
    LogWarn("type not match");
    return false;
}

bool SockAddr::operator == (const SockAddr &rhs) const
{
    if (len_ != rhs.len_)
        return false;

    return ::memcmp(&addr_, &rhs.addr_, len_) == 0;
}

}
}
