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
    fd_.setNonBlock(true);
    sp_ssl_->setFd(fd_.get());

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

    recv_buff_.ensureWritableSize(1024);
    int rsize = sp_ssl_->read(recv_buff_.writableBegin(), recv_buff_.writableSize());
    int err = sp_ssl_->getError(rsize);
    if (rsize > 0) {
        recv_buff_.hasWritten(rsize);
        if (wp_receiver_ != nullptr) {
            wp_receiver_->send(recv_buff_.readableBegin(), recv_buff_.readableSize());
            recv_buff_.hasReadAll();

        } else if (recv_buff_.readableSize() >= recv_threshold_) {
            if (recv_cb_) {
                ++cb_level_;
                recv_cb_(recv_buff_);
                --cb_level_;
            } else {
                LogWarn("recv_cb_ is not set");
                recv_buff_.hasReadAll();    //! 丢弃数据，防止堆积
            }
        }
    } else if (rsize == 0) {
        if (err == SSL_ERROR_ZERO_RETURN) {
            onSocketClosed();
        }
    } else {
        if (errno != EAGAIN) {
            LogWarn("read error, rsize:%d, errno:%d, %s", rsize, errno, strerror(errno));
        }
    }
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
    int ret = sp_ssl_->doHandshake();
    if (ret == 1) {
        is_ssl_done_ = true;
        return;
    }

    int err = sp_ssl_->getError(ret);
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

bool TcpSslConnection::send(const void *data_ptr, size_t data_size)
{
    auto wsize = sp_ssl_->write(data_ptr, data_size);
    if (wsize > 0) {
        if (static_cast<size_t>(wsize) == data_size)
            return true;
        else
            LogWarn("wsize != data_size, wsize:%d, data_size:%u", wsize, data_size);
    } else if (wsize < 0) {
        int err = sp_ssl_->getError(wsize);
        LogWarn("SSL write err:%d, errno:%d, %s", err, errno, strerror(errno));
    } else {
        LogWarn("SSL wsize = 0");
    }

    return false;
}

void TcpSslConnection::onSocketClosed()
{
    LogInfo("%s", peer_addr_.toString().c_str());

    sp_read_event_->disable();
    if (disconnected_cb_) {
        ++cb_level_;
        disconnected_cb_();
        --cb_level_;
    }
}

}
}
