#include "ip_address.h"

#include <arpa/inet.h>

namespace tbox {
namespace network {

IPAddress::IPAddress(const std::string &ip_str)
{
    ip_ = inet_addr(ip_str.c_str());
}

std::string IPAddress::toString() const
{
    char ip_str_buff[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &ip_, ip_str_buff, INET_ADDRSTRLEN);
    return ip_str_buff;
}

}
}
