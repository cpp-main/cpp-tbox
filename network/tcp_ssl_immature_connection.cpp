#include "tcp_ssl_immature_connection.h"

#include <tbox/base/defines.h>

namespace tbox {
namespace network {
TcpSslImmatureConnection::TcpSslImmatureConnection(
        event::Loop *wp_loop, SocketFd fd, SslCtx *wp_ssl_ctx, const SockAddr &peer_addr) :
    wp_loop_(wp_loop),
    sp_ssl_(wp_ssl_ctx->newSsl()),
    peer_addr_(peer_addr),
    sp_read_event_(wp_loop->newFdEvent()),
    sp_write_event_(wp_loop->newFdEvent())
{
    sp_read_event_->initialize(fd.get(), event::FdEvent::kReadEvent, event::Event::Mode::kPersist);
    sp_write_event_->initialize(fd.get(), event::FdEvent::kWriteEvent, event::Event::Mode::kOneshot);
}

TcpSslImmatureConnection::~TcpSslImmatureConnection()
{
    assert(cb_level_ == 0);
    CHECK_DELETE_RESET_OBJ(sp_write_event_);
    CHECK_DELETE_RESET_OBJ(sp_read_event_);
    CHECK_DELETE_RESET_OBJ(sp_ssl_);
}

}
}
