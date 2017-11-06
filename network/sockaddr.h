#ifndef TBOX_NETWORK_SOCKADDR_H_20171105
#define TBOX_NETWORK_SOCKADDR_H_20171105

#include <sys/socket.h>
#include <netinet/in.h>
#include <string>

#include "ip_address.h"

namespace tbox {
namespace network {

class SockAddr {
  public:
    SockAddr();
    SockAddr(const struct sockaddr &addr, socklen_t len);
    SockAddr(const struct sockaddr_in &addr_in);

    SockAddr(const SockAddr &other);
    SockAddr& operator = (const SockAddr &other);

    static SockAddr FromString(const std::string &add_str);

    enum Type {
        kNone,
        kIPv4,
        kLocal, //! Unix Local socket
    };
    Type type() const;
    std::string toString() const;

    bool get(IPAddress &ip, uint16_t &port) const;

    socklen_t toSockAddr(struct sockaddr &addr) const;
    socklen_t toSockAddr(struct sockaddr_in &addr_in) const;
    socklen_t toSockAddr(struct sockaddr_un &addr_un) const;

    bool operator == (const SockAddr &rhs) const;
    bool operator != (const SockAddr &rhs) const { return !(*this == rhs); }

  protected:
    SockAddr(const IPAddress &ip, uint16_t port);
    SockAddr(const std::string &sock_path);

  private:
    struct sockaddr_storage addr_;
    socklen_t len_ = 0; //! 当地址为 AF_LOCAL 时，表示字串长度
};

}
}

#endif //TBOX_NETWORK_SOCKADDR_H_20171105
