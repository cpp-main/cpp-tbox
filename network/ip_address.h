#ifndef TBOX_NETWORK_IP_ADDRESS_H_20171105
#define TBOX_NETWORK_IP_ADDRESS_H_20171105

#include <cstdint>
#include <string>

namespace tbox {
namespace network {

class IPAddress {
  public:
    IPAddress(uint32_t ip = 0) : ip_(ip) { }
    IPAddress(const std::string &ip_str);

    IPAddress& operator = (uint32_t ip) { ip_ = ip; return *this; }
    inline operator uint32_t () const { return ip_; }
    inline operator std::string () const { return toString(); }

    IPAddress operator ~ () const  { return IPAddress(~ip_); }
    IPAddress operator & (const IPAddress &rhs) const { return IPAddress(ip_ & rhs.ip_); }
    IPAddress operator | (const IPAddress &rhs) const { return IPAddress(ip_ | rhs.ip_); }

    bool operator == (const IPAddress &rhs) const { return ip_ == rhs.ip_; }

    std::string toString() const;

  public:
    const static IPAddress Any;

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
