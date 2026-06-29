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
#ifndef TBOX_NETWORK_TCP_RAW_FACTORY_H_20260616
#define TBOX_NETWORK_TCP_RAW_FACTORY_H_20260616

#include "tcp_factory.h"

namespace tbox {
namespace network {

//! 原始 TCP 工厂（无 TLS）
//! 创建 TcpRawConnector 和 TcpRawAcceptor
class TcpRawFactory : public TcpFactory {
  public:
    virtual bool initialize() override { return true; }
    virtual TcpConnector* createConnector(event::Loop *wp_loop) override;
    virtual TcpAcceptor*  createAcceptor(event::Loop *wp_loop) override;
};

}
}
#endif //TBOX_NETWORK_TCP_RAW_FACTORY_H_20260616
