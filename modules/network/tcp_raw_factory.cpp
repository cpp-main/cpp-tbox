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
#include "tcp_raw_factory.h"
#include "tcp_raw_connector.h"
#include "tcp_raw_acceptor.h"

namespace tbox {
namespace network {

TcpConnector* TcpRawFactory::createConnector(event::Loop *wp_loop)
{
    return new TcpRawConnector(wp_loop);
}

TcpAcceptor* TcpRawFactory::createAcceptor(event::Loop *wp_loop)
{
    return new TcpRawAcceptor(wp_loop);
}

}
}
