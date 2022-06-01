#include "tcp_ssl_acceptor.h"

#include <sys/un.h>
#include <unistd.h>
#include <errno.h>
#include <cstring>

#include <tbox/base/log.h>

#include "tcp_ssl_connection.h"

namespace tbox {
namespace network {

TcpSslAcceptor::TcpSslAcceptor(event::Loop *wp_loop) :
    wp_loop_(wp_loop),
    sp_ssl_ctx_(new SslCtx)
{ }

TcpSslAcceptor::~TcpSslAcceptor()
{
    assert(cb_level_ == 0);
    if (sp_read_ev_ != nullptr)
        cleanup();

    CHECK_DELETE_RESET_OBJ(sp_ssl_ctx_);
}

bool TcpSslAcceptor::initialize(const SockAddr &bind_addr, int listen_backlog)
{
    LogDbg("bind_addr:%s, backlog:%d", bind_addr.toString().c_str(), listen_backlog);

    if (bind_addr.type() != SockAddr::Type::kIPv4) {
        LogWarn("TcpSsl only support IPv4 and IPv6");
        return false;
    }

    bind_addr_ = bind_addr;

    SocketFd sock_fd = createSocket(bind_addr.type());
    if (sock_fd.isNull()) {
        LogErr("create socket fail");
        return false;
    }

    int bind_ret = bindAddress(sock_fd, bind_addr);
    if (bind_ret < 0) {
        LogErr("bind address %s fail", bind_addr.toString().c_str());
        return false;
    }

    int listen_ret = sock_fd.listen(listen_backlog);
    if (listen_ret < 0) {
        LogErr("listen fail");
        return false;
    }

    sock_fd_ = sock_fd;
    CHECK_DELETE_RESET_OBJ(sp_read_ev_);
    sp_read_ev_ = wp_loop_->newFdEvent();
    sp_read_ev_->initialize(sock_fd.get(), event::FdEvent::kReadEvent, event::Event::Mode::kPersist);
    sp_read_ev_->setCallback(std::bind(&TcpSslAcceptor::onSocketRead, this, std::placeholders::_1));

    return true;
}

bool TcpSslAcceptor::useCertificateFile(const std::string &filename, int filetype)
{
    if (sp_ssl_ctx_->useCertificateFile(filename, filetype)) {
        ssl_setting_bits |= 1;
        return true;
    }
    return false;
}

bool TcpSslAcceptor::usePrivateKeyFile(const std::string &filename, int filetype)
{
    if (sp_ssl_ctx_->usePrivateKeyFile(filename, filetype)) {
        ssl_setting_bits |= 2;
        return true;
    }
    return false;
}

bool TcpSslAcceptor::checkPrivateKey()
{
    if ((ssl_setting_bits & 0x03) != 0x03) {
        LogWarn("use Certificate and private file first.");
        return false;
    }

    if (sp_ssl_ctx_->checkPrivateKey()) {
        ssl_setting_bits |= 4;
        return true;
    }
    return false;
}

SocketFd TcpSslAcceptor::createSocket(SockAddr::Type addr_type)
{
    int flags = SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC;
    if (addr_type == SockAddr::Type::kIPv4)
        return SocketFd::CreateSocket(AF_INET, flags, 0);
    else if (addr_type == SockAddr::Type::kLocal)
        return SocketFd::CreateSocket(AF_LOCAL, flags, 0);
    else {
        LogErr("socket type not support");
        return SocketFd();
    }
}

int TcpSslAcceptor::bindAddress(SocketFd sock_fd, const SockAddr &bind_addr)
{
    if (bind_addr.type() == SockAddr::Type::kIPv4) {
        struct sockaddr_in sock_addr;
        socklen_t len = bind_addr.toSockAddr(sock_addr);
        return sock_fd.bind((const struct sockaddr*)&sock_addr, len);
    } else if (bind_addr.type() == SockAddr::Type::kLocal) {
        struct sockaddr_un sock_addr;
        socklen_t len = bind_addr.toSockAddr(sock_addr);
        return sock_fd.bind((const struct sockaddr*)&sock_addr, len);
    } else {
        LogErr("bind addr type not support");
        return -1;
    }
}

bool TcpSslAcceptor::start()
{
    if (ssl_setting_bits != 0x07) {
        LogWarn("SSL not set.");
        return false;
    }

    if (sp_read_ev_ != nullptr)
        return sp_read_ev_->enable();

    return false;
}

bool TcpSslAcceptor::stop()
{
    if (sp_read_ev_ != nullptr)
        return sp_read_ev_->disable();
    return false;
}

void TcpSslAcceptor::cleanup()
{
    CHECK_DELETE_RESET_OBJ(sp_read_ev_);
    unfinished_conn_.foreach([](TcpSslConnection *conn) { delete conn; });
    sock_fd_.close();
}

void TcpSslAcceptor::onSocketRead(short events)
{
    if (events & event::FdEvent::kReadEvent)
        onClientConnected();
}

void TcpSslAcceptor::onClientConnected()
{
    struct sockaddr addr;
    socklen_t addr_len = sizeof(addr);
    SocketFd peer_sock = sock_fd_.accept(&addr, &addr_len);
    if (peer_sock.isNull()) {
        LogWarn("accept fail");
        return;
    }
    SockAddr peer_addr(addr, addr_len);
    LogInfo("%s accepted new connection: %s", bind_addr_.toString().c_str(), peer_addr.toString().c_str());

    auto new_conn = new TcpSslConnection(wp_loop_, peer_sock, sp_ssl_ctx_, peer_addr, true);
    auto new_token = unfinished_conn_.alloc(new_conn);
    new_conn->setSslFinishedCallback([new_token, this] () { onClientSslFinished(new_token); });
}

void TcpSslAcceptor::onClientSslFinished(const cabinet::Token &token)
{
    auto new_conn = unfinished_conn_.free(token);
    if (new_conn->isSslDone()) {
        LogInfo("connection: %s", new_conn->peerAddr().toString().c_str());
        if (new_conn_cb_) {
            ++cb_level_;
            new_conn_cb_(new_conn);
            --cb_level_;
        }
    } else {
        wp_loop_->runNext([new_conn] { delete new_conn; });
    }
}

}
}
