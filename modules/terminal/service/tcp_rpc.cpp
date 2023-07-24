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
#include "tcp_rpc.h"
#include "../impl/service/tcp_rpc.h"
#include <tbox/base/assert.h>

namespace tbox {
namespace terminal {

TcpRpc::TcpRpc(event::Loop *wp_loop, TerminalInteract *wp_terminal) :
    impl_(new Impl(wp_loop, wp_terminal))
{
    TBOX_ASSERT(impl_ != nullptr);
}

TcpRpc::~TcpRpc()
{
    delete impl_;
}

bool TcpRpc::initialize(const std::string &bind_addr)
{
    return impl_->initialize(bind_addr);
}

bool TcpRpc::start()
{
    return impl_->start();
}

void TcpRpc::stop()
{
    return impl_->stop();
}

void TcpRpc::cleanup()
{
    return impl_->cleanup();
}

}
}
