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
