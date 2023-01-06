#ifndef TBOX_NETWORK_NET_IF_H_20221226
#define TBOX_NETWORK_NET_IF_H_20221226

#include <string>
#include <vector>

#include "ip_address.h"

namespace tbox {
namespace network {

struct NetIF {
    std::string name;
    IPAddress   ip;
    IPAddress   mask;
    uint32_t    flags = 0;
};

bool GetNetIF(std::vector<NetIF> &net_if_vec);
bool GetNetIF(const std::string &name, std::vector<NetIF> &net_if_vec);

}
}

#endif //TBOX_NETWORK_NET_IF_H_20221226
