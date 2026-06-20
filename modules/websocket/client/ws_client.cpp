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
 * Copyright (c) 2026 Hevake and contributors, all rights reserved.
 *
 * This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
 * Use of this source code is governed by MIT license that can be found
 * in the LICENSE file in the root of the source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */
#include "ws_client.h"
#include "ws_client_impl.h"
#include <tbox/base/assert.h>

namespace tbox {
namespace websocket {
namespace client {

WsClient::WsClient(event::Loop *wp_loop)
  : impl_(new Impl(this, wp_loop))
{
    TBOX_ASSERT(wp_loop != nullptr);
}

WsClient::~WsClient()
{
    CHECK_DELETE_RESET_OBJ(impl_);
}

bool WsClient::initialize(const network::SockAddr &server_addr, const std::string &url_path)
{
    return impl_->initialize(server_addr, url_path);
}

bool WsClient::start()
{
    return impl_->start();
}

void WsClient::stop()
{
    impl_->stop();
}

void WsClient::cleanup()
{
    impl_->cleanup();
}

WsClient::State WsClient::state() const
{
    return impl_->state();
}

void WsClient::setConnectedCallback(const ConnectedCallback &cb)
{
    impl_->setConnectedCallback(cb);
}

void WsClient::setDisconnectedCallback(const DisconnectedCallback &cb)
{
    impl_->setDisconnectedCallback(cb);
}

void WsClient::setMessageCallback(const MessageCallback &cb)
{
    impl_->setMessageCallback(cb);
}

void WsClient::setErrorCallback(const ErrorCallback &cb)
{
    impl_->setErrorCallback(cb);
}

void WsClient::setAutoReconnect(bool enable)
{
    impl_->setAutoReconnect(enable);
}

void WsClient::setReconnectDelayCalcFunc(const ReconnectDelayCalc &func)
{
    impl_->setReconnectDelayCalcFunc(func);
}

bool WsClient::send(const std::string &text)
{
    return impl_->send(text);
}

bool WsClient::send(const void *data, size_t len)
{
    return impl_->send(data, len);
}

bool WsClient::sendBinary(const std::vector<uint8_t> &data)
{
    return impl_->sendBinary(data);
}

bool WsClient::close(uint16_t code, const std::string &reason)
{
    return impl_->close(code, reason);
}

bool WsClient::ping(const std::string &data)
{
    return impl_->ping(data);
}

bool WsClient::pong(const std::string &data)
{
    return impl_->pong(data);
}

bool WsClient::isExpired() const
{
    return impl_->isExpired();
}

network::SockAddr WsClient::peerAddr() const
{
    return impl_->peerAddr();
}

void WsClient::setContext(void *context, ContextDeleter &&deleter)
{
    impl_->setContext(context, std::move(deleter));
}

void* WsClient::getContext() const
{
    return impl_->getContext();
}

}
}
}
