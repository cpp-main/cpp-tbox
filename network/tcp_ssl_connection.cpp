#include "tcp_ssl_connection.h"

#include <cassert>
#include <tbox/base/log.h>

#include "tcp_ssl_immature_connection.h"

namespace tbox {
namespace network {

using namespace std::placeholders;

TcpSslConnection::TcpSslConnection(TcpSslImmatureConnection &conn) :
    wp_loop_(conn.wp_loop_),
    sp_ssl_(conn.sp_ssl_),
    peer_addr_(conn.peer_addr_),
    sp_read_event_(conn.sp_read_event_),
    sp_write_event_(conn.sp_write_event_)
{
    conn.sp_read_event_ = nullptr;
    conn.sp_write_event_ = nullptr;
}

TcpSslConnection::~TcpSslConnection()
{
    assert(cb_level_ == 0);
    CHECK_DELETE_RESET_OBJ(sp_write_event_);
    CHECK_DELETE_RESET_OBJ(sp_read_event_);
    CHECK_DELETE_RESET_OBJ(sp_ssl_);
}

void TcpSslConnection::enable()
{
    LogUndo();
}

bool TcpSslConnection::disconnect()
{
    LogUndo();
    return false;
}

bool TcpSslConnection::shutdown(int howto)
{
    LogUndo();
    return false;
}

void* TcpSslConnection::setContext(void *new_context)
{
    auto old_context = wp_context_;
    wp_context_ = new_context;
    return old_context;
}

void TcpSslConnection::setReceiveCallback(const ReceiveCallback &cb, size_t threshold)
{
    LogUndo();
}

void TcpSslConnection::bind(ByteStream *receiver)
{
    LogUndo();
}

void TcpSslConnection::unbind()
{
    LogUndo();
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
