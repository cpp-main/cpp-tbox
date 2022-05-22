#include "tcp_ssl_client.h"

#include <tbox/base/log.h>
#include <tbox/base/defines.h>

#include "tcp_ssl_connector.h"
#include "tcp_ssl_connection.h"

namespace tbox {
namespace network {

struct TcpSslClient::Data {
    event::Loop *wp_loop;
    State state = State::kNone;

    ConnectedCallback    connected_cb;
    DisconnectedCallback disconnected_cb;
    ReceiveCallback      received_cb;
    size_t               received_threshold = 0;
    ByteStream          *wp_receiver = nullptr;
    bool reconnect_enabled = true;

    TcpSslConnector  *sp_connector  = nullptr;
    TcpSslConnection *sp_connection = nullptr;

    int cb_level = 0;
};

TcpSslClient::TcpSslClient(event::Loop *wp_loop) :
    d_(new Data)
{
    assert(d_ != nullptr);

    d_->wp_loop = wp_loop;
    d_->sp_connector = new TcpSslConnector(wp_loop);
}

TcpSslClient::~TcpSslClient()
{
    assert(d_->cb_level == 0);

    cleanup();

    CHECK_DELETE_RESET_OBJ(d_->sp_connection);
    CHECK_DELETE_RESET_OBJ(d_->sp_connector);

    delete d_;
}

bool TcpSslClient::initialize(const SockAddr &server_addr)
{
    if (d_->state != State::kNone) {
        LogWarn("not in none state, cleanup first.");
        return false;
    }

    using namespace std::placeholders;
    d_->sp_connector->initialize(server_addr);
    d_->sp_connector->setConnectedCallback(std::bind(&TcpSslClient::onTcpConnected, this, _1));

    d_->state = State::kInited;
    return true;
}

void TcpSslClient::setConnectedCallback(const ConnectedCallback &cb)
{
    d_->connected_cb = cb;
}

void TcpSslClient::setDisconnectedCallback(const DisconnectedCallback &cb)
{
    d_->disconnected_cb = cb;
}

void TcpSslClient::setAutoReconnect(bool enable)
{
    d_->reconnect_enabled = enable;
}

bool TcpSslClient::start()
{
    if (d_->state != State::kInited) {
        LogWarn("not in idle state, initialize or stop first");
        return false;
    }

    d_->state = State::kConnecting;
    return d_->sp_connector->start();
}

void TcpSslClient::stop()
{
    if (d_->state == State::kConnecting) {
        d_->state = State::kInited;
        d_->sp_connector->stop();

    } else if (d_->state == State::kConnected) {
        TcpSslConnection *tobe_delete = nullptr;
        std::swap(tobe_delete, d_->sp_connection);
        tobe_delete->disconnect();
        d_->wp_loop->runNext([tobe_delete] { delete tobe_delete; });
        d_->state = State::kInited;
    }
}

bool TcpSslClient::shutdown(int howto)
{
    if (d_->state == State::kConnected)
        return d_->sp_connection->shutdown(howto);
    return false;
}

void TcpSslClient::cleanup()
{
    if (d_->state <= State::kNone)
        return;

    stop();

    d_->sp_connector->cleanup();

    d_->connected_cb = nullptr;
    d_->disconnected_cb = nullptr;
    d_->received_cb = nullptr;
    d_->received_threshold = 0;
    d_->wp_receiver = nullptr;
    d_->reconnect_enabled = true;

    d_->state = State::kNone;
}

TcpSslClient::State TcpSslClient::state() const
{
    return d_->state;
}

void TcpSslClient::setReceiveCallback(const ReceiveCallback &cb, size_t threshold)
{
    if (d_->sp_connection != nullptr)
        d_->sp_connection->setReceiveCallback(cb, threshold);

    d_->received_cb = cb;
    d_->received_threshold = threshold;
}

bool TcpSslClient::send(const void *data_ptr, size_t data_size)
{
    if (d_->sp_connection != nullptr)
        return d_->sp_connection->send(data_ptr, data_size);

    return false;
}

void TcpSslClient::bind(ByteStream *receiver)
{
    if (d_->sp_connection != nullptr)
        d_->sp_connection->bind(receiver);

    d_->wp_receiver = receiver;
}

void TcpSslClient::unbind()
{
    if (d_->sp_connection != nullptr)
        d_->sp_connection->unbind();

    d_->wp_receiver = nullptr;
}

void TcpSslClient::onTcpConnected(TcpSslConnection *new_conn)
{
    new_conn->setDisconnectedCallback(std::bind(&TcpSslClient::onTcpDisconnected, this));
    new_conn->setReceiveCallback(d_->received_cb, d_->received_threshold);
    if (d_->wp_receiver != nullptr)
        new_conn->bind(d_->wp_receiver);

    //! 可以直接释放，因为本函数一定是 d_->sp_connector 对象调用的
    CHECK_DELETE_RESET_OBJ(d_->sp_connection);
    d_->sp_connection = new_conn;

    d_->state = State::kConnected;

    if (d_->connected_cb) {
        ++d_->cb_level;
        d_->connected_cb();
        --d_->cb_level;
    }
}

void TcpSslClient::onTcpDisconnected()
{
    TcpSslConnection *tobe_delete = nullptr;
    std::swap(tobe_delete, d_->sp_connection);
    //! 这里要使用延后释放，因为本函数一定是 d_->sp_connection 对象自己调用的
    d_->wp_loop->runNext([tobe_delete] { delete tobe_delete; });

    d_->state = State::kInited;

    if (d_->reconnect_enabled)
        start();

    if (d_->disconnected_cb) {
        ++d_->cb_level;
        d_->disconnected_cb();
        --d_->cb_level;
    }
}

}
}
