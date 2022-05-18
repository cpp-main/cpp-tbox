#include "tcp_connection.h"

#include <cassert>
#include <tbox/base/log.h>

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
    sp_buffered_fd_->setErrorCallback(std::bind(&TcpConnection::onError, this, _1));

    sp_buffered_fd_->enable();
}

TcpConnection::~TcpConnection()
{
    assert(cb_level_ == 0);
    CHECK_DELETE_RESET_OBJ(sp_buffered_fd_);
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
    wp_loop_->runNext([tmp] { CHECK_DELETE_OBJ(tmp); });

    return true;
}

bool TcpConnection::shutdown(int howto)
{
    LogInfo("%s, %d", peer_addr_.toString().c_str(), howto);
    if (sp_buffered_fd_ == nullptr)
        return false;

    SocketFd socket_fd = static_cast<SocketFd>(sp_buffered_fd_->fd());
    return socket_fd.shutdown(howto) == 0;
}

SocketFd TcpConnection::socketFd() const
{
    if (sp_buffered_fd_ != nullptr)
        return static_cast<SocketFd>(sp_buffered_fd_->fd());
    return SocketFd();
}

void* TcpConnection::setContext(void *new_context)
{
    auto old_context = wp_context_;
    wp_context_ = new_context;
    return old_context;
}

void TcpConnection::setReceiveCallback(const ReceiveCallback &cb, size_t threshold)
{
    if (sp_buffered_fd_ != nullptr)
        sp_buffered_fd_->setReceiveCallback(cb, threshold);
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

void TcpConnection::onSocketClosed()
{
    LogInfo("%s", peer_addr_.toString().c_str());

    BufferedFd *tmp = nullptr;
    std::swap(tmp, sp_buffered_fd_);
    wp_loop_->runNext([tmp] { CHECK_DELETE_OBJ(tmp); });

    if (disconnected_cb_) {
        ++cb_level_;
        disconnected_cb_();
        --cb_level_;
    }
}

void TcpConnection::onError(int errnum)
{
    if (errnum == ECONNRESET) {
        onSocketClosed();
    } else {
        LogWarn("errno:%d, %s", errnum, strerror(errnum));
    }
}

}
}
