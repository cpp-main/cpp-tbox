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
#ifndef TBOX_NETWORK_TCP_FACTORY_H_20260616
#define TBOX_NETWORK_TCP_FACTORY_H_20260616

#include <tbox/event/forward.h>

namespace tbox {
namespace network {

class TcpConnector;
class TcpAcceptor;

//! TCP 抽象工厂，用于创建 Connector 和 Acceptor
//! TcpServer 和 TcpClient 通过工厂决定使用 raw 还是 TLS 传输
class TcpFactory {
  public:
    virtual ~TcpFactory() {}
    virtual bool initialize() = 0;

    virtual TcpConnector* createConnector(event::Loop *wp_loop) = 0;
    virtual TcpAcceptor*  createAcceptor(event::Loop *wp_loop) = 0;
};

}
}
#endif //TBOX_NETWORK_TCP_FACTORY_H_20260616
