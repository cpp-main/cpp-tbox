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
#ifndef TBOX_NETWORK_IP_ADDRESS_H_20171105
#define TBOX_NETWORK_IP_ADDRESS_H_20171105

#include <cstdint>
#include <string>
#include <stdexcept>

namespace tbox {
namespace network {

class IPAddress {
  public:
    struct FormatInvalid : public std::runtime_error {
        FormatInvalid(const std::string &str) : std::runtime_error(Format(str)) { }
        static std::string Format(const std::string &str);
    };

    explicit IPAddress(uint32_t ip = 0) : ip_(ip) { }

    IPAddress& operator = (uint32_t ip) { ip_ = ip; return *this; }
    inline operator uint32_t () const { return ip_; }
    inline operator std::string () const { return toString(); }

    IPAddress operator ~ () const  { return IPAddress(~ip_); }
    IPAddress operator & (const IPAddress &rhs) const { return IPAddress(ip_ & rhs.ip_); }
    IPAddress operator | (const IPAddress &rhs) const { return IPAddress(ip_ | rhs.ip_); }

    bool operator == (const IPAddress &rhs) const { return ip_ == rhs.ip_; }

    std::string toString() const;

  public:
    /// 将字串转换成IPAddress
    /**
     * \param   ip_str      字串
     * \return  IPAddress
     *
     * \throw   FormatInvalid   字串格式不对
     */
    static IPAddress FromString(const std::string &ip_str);

    static IPAddress Any();     //! 0.0.0.0
    static IPAddress Loop();    //! 127.0.0.1

  private:
    uint32_t ip_;
};

//!TODO
class IPv6Address {
  public:
    IPv6Address();
  private:
    uint16_t ipv6_[8] = {0};
};
}
}

#endif //TBOX_NETWORK_IP_ADDRESS_H_20171105
