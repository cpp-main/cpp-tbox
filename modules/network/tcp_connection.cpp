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
#include "tcp_connection.h"

#include <tbox/base/log.h>
#include <tbox/base/assert.h>

namespace tbox {
namespace network {

using namespace std::placeholders;

TcpConnection::TcpConnection(event::Loop *wp_loop, SocketFd fd, const SockAddr &peer_addr) :
    wp_loop_(wp_loop),
    sp_buffered_fd_(new BufferedFd(wp_loop)),
    peer_addr_(peer_addr)
{
    sp_buffered_fd_->initialize(fd);
    sp_buffered_fd_->setReadZeroCallback(std::bind(&TcpConnection::onSocketClosed, this));
    sp_buffered_fd_->setReadErrorCallback(std::bind(&TcpConnection::onReadError, this, _1));

    sp_buffered_fd_->enable();
}

TcpConnection::~TcpConnection()
{
    TBOX_ASSERT(cb_level_ == 0);
    CHECK_DELETE_RESET_OBJ(sp_buffered_fd_);

    if (sp_context_ != nullptr && context_deleter_)
        context_deleter_(sp_context_);
}

void TcpConnection::enable()
{
    sp_buffered_fd_->enable();
}

bool TcpConnection::disconnect()
{
    LogInfo("%s", peer_addr_.toString().c_str());
    if (sp_buffered_fd_ == nullptr)
        return false;

    sp_buffered_fd_->disable();

    BufferedFd *tmp = nullptr;
    std::swap(tmp, sp_buffered_fd_);

    wp_loop_->runNext(
        [tmp] { CHECK_DELETE_OBJ(tmp); },
        "TcpConnection::disconnect, delete tmp"
    );

    return true;
}

bool TcpConnection::shutdown(int howto)
{
    LogInfo("%s, %d", peer_addr_.toString().c_str(), howto);
    if (sp_buffered_fd_ == nullptr)
        return false;

    SocketFd socket_fd(sp_buffered_fd_->fd());
    return socket_fd.shutdown(howto) == 0;
}

SocketFd TcpConnection::socketFd() const
{
    if (sp_buffered_fd_ != nullptr)
        return sp_buffered_fd_->fd();
    return SocketFd();
}

void TcpConnection::setContext(void *context, ContextDeleter &&deleter)
{
    if (sp_context_ != nullptr && context_deleter_)
        context_deleter_(sp_context_);

    sp_context_ = context;
    context_deleter_ = std::move(deleter);
}

void TcpConnection::setReceiveCallback(const ReceiveCallback &cb, size_t threshold)
{
    if (sp_buffered_fd_ != nullptr)
        sp_buffered_fd_->setReceiveCallback(cb, threshold);
}

void TcpConnection::setSendCompleteCallback(const SendCompleteCallback &cb)
{
    if (sp_buffered_fd_ != nullptr)
        sp_buffered_fd_->setSendCompleteCallback(cb);
}

void TcpConnection::bind(ByteStream *receiver)
{
    if (sp_buffered_fd_ != nullptr)
        sp_buffered_fd_->bind(receiver);
}

void TcpConnection::unbind()
{
    if (sp_buffered_fd_ != nullptr)
        sp_buffered_fd_->unbind();
}

bool TcpConnection::send(const void *data_ptr, size_t data_size)
{
    if (sp_buffered_fd_ != nullptr)
        return sp_buffered_fd_->send(data_ptr, data_size);
    return false;
}

Buffer* TcpConnection::getReceiveBuffer()
{
    if (sp_buffered_fd_ != nullptr)
        return sp_buffered_fd_->getReceiveBuffer();
    return nullptr;
}

void TcpConnection::onSocketClosed()
{
    LogInfo("%s", peer_addr_.toString().c_str());

    sp_buffered_fd_->disable();

    BufferedFd *tmp = nullptr;
    std::swap(tmp, sp_buffered_fd_);

    wp_loop_->runNext(
        [tmp] { CHECK_DELETE_OBJ(tmp); },
        "TcpConnection::onSocketClosed, delete tmp"
    );

    if (disconnected_cb_) {
        ++cb_level_;
        disconnected_cb_();
        --cb_level_;
    }
}

void TcpConnection::onReadError(int errnum)
{
    LogNotice("errno:%d, %s", errnum, strerror(errnum));
    onSocketClosed();
}

}
}
