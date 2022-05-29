#include "tcp_ssl_connection.h"

#include <errno.h>
#include <cassert>
#include <tbox/base/log.h>

namespace tbox {
namespace network {

using namespace std::placeholders;

TcpSslConnection::TcpSslConnection(
        event::Loop *wp_loop, SocketFd fd, SslCtx *wp_ssl_ctx,
        const SockAddr &peer_addr, bool is_accept_state) :
    wp_loop_(wp_loop),
    fd_(fd),
    sp_ssl_(wp_ssl_ctx->newSsl()),
    peer_addr_(peer_addr),
    sp_read_event_(wp_loop->newFdEvent()),
    sp_write_event_(wp_loop->newFdEvent())
{
    if (is_accept_state)
        sp_ssl_->setAcceptState();
    else
        sp_ssl_->setConnectState();

    sp_read_event_->initialize(fd.get(), event::FdEvent::kReadEvent, event::Event::Mode::kPersist);
    sp_read_event_->setCallback(std::bind(&TcpSslConnection::onFdReadable, this, _1));
    sp_read_event_->enable();

    sp_write_event_->initialize(fd.get(), event::FdEvent::kWriteEvent, event::Event::Mode::kOneshot);
    sp_write_event_->setCallback(std::bind(&TcpSslConnection::onFdWritable, this, _1));
}

TcpSslConnection::~TcpSslConnection()
{
    assert(cb_level_ == 0);
    CHECK_DELETE_RESET_OBJ(sp_write_event_);
    CHECK_DELETE_RESET_OBJ(sp_read_event_);
    CHECK_DELETE_RESET_OBJ(sp_ssl_);
}

void TcpSslConnection::onFdReadable(short)
{
    if (!is_ssl_done_) {
        doHandshake();
        return;
    }
    LogUndo();  //TODO
}

void TcpSslConnection::onFdWritable(short)
{
    if (!is_ssl_done_) {
        doHandshake();
        return;
    }
    LogUndo();  //TODO
}

void TcpSslConnection::doHandshake()
{
    int r = sp_ssl_->doHandshake();
    if (r == 1) {
        is_ssl_done_ = true;
        return;
    }

    int err = sp_ssl_->getError(r);
    if (err == SSL_ERROR_WANT_WRITE) {
        sp_write_event_->enable();
    } else if (err == SSL_ERROR_WANT_READ) {
        //! nothing todo
    } else {
        LogNotice("SSL handshake err:%d, errno:%d, %s", err, errno, strerror(errno));
    }
}

bool TcpSslConnection::disconnect()
{
    sp_read_event_->disable();
    sp_write_event_->disable();
    fd_.reset();
    return true;
}

bool TcpSslConnection::shutdown(int howto)
{
    return fd_.shutdown(howto) == 0;
}

void* TcpSslConnection::setContext(void *new_context)
{
    auto old_context = wp_context_;
    wp_context_ = new_context;
    return old_context;
}

void TcpSslConnection::setReceiveCallback(const ReceiveCallback &cb, size_t threshold)
{
    recv_cb_ = cb;
    recv_threshold_ = threshold;
}

void TcpSslConnection::bind(ByteStream *receiver)
{
    wp_stream_ = receiver;
}

void TcpSslConnection::unbind()
{
    wp_stream_ = nullptr;
}

bool TcpSslConnection::send(const void *data_ptr, size_t data_size)
{
    LogUndo();
    return false;
}

void TcpSslConnection::onSocketClosed()
{
    LogInfo("%s", peer_addr_.toString().c_str());

    LogUndo();

    if (disconnected_cb_) {
        ++cb_level_;
        disconnected_cb_();
        --cb_level_;
    }
}

}
}
