#include "tcp_connector.h"

#include <sys/un.h>
#include <cassert>

#include <tbox/base/log.h>

namespace tbox {
namespace network {

TcpConnector::TcpConnector(event::Loop *wp_loop) :
    wp_loop_(wp_loop)
{ }

TcpConnector::~TcpConnector()
{
    assert(cb_level_ == 0);

    CHECK_DELETE_RESET_OBJ(sp_delay_ev_);
    CHECK_DELETE_RESET_OBJ(sp_write_ev_);
}

void TcpConnector::checkSettingAndTryEnterIdleState()
{
    if (state_ == State::kNone) {
        if (server_addr_.type() != SockAddr::Type::kNone && connected_cb_)
            state_ = State::kIdle;
    }
}

void TcpConnector::initialize(const SockAddr &server_addr)
{
    server_addr_ = server_addr;
    checkSettingAndTryEnterIdleState();
}

void TcpConnector::setConnectedCallback(const ConnectedCallback &cb)
{
    connected_cb_ = cb;
    checkSettingAndTryEnterIdleState();
}

void TcpConnector::setTryTimes(int try_times)
{
    try_times_ = try_times;
}

void TcpConnector::setConnectFailCallback(const ConnectFailCallback &cb)
{
    connect_fail_cb_ = cb;
}

void TcpConnector::setReconnectDelayCalcFunc(const ReconnectDelayCalc &func)
{
    reconn_delay_calc_func_ = func;
}

bool TcpConnector::start()
{
    if (state_ != State::kIdle) {
        LogWarn("not in idle state");
        return false;
    }

    conn_fail_times_ = 0;
    enterConnectingState();
    return true;
}

bool TcpConnector::stop()
{
    if ((state_ == State::kConnecting) || (state_ == State::kNone))
        return false;

    if (state_ == State::kConnecting)
        exitConnectingState();
    else if (state_ == State::kReconnectDelay)
        exitReconnectDelayState();

    state_ = State::kIdle;
    return true;
}

SocketFd TcpConnector::createSocket(SockAddr::Type addr_type) const
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

int TcpConnector::connect(SocketFd sock_fd, const SockAddr &addr) const
{
    int ret = -1;

    if (addr.type() == SockAddr::Type::kIPv4) {
        struct sockaddr_in conn_addr;
        socklen_t addr_len = addr.toSockAddr(conn_addr);
        ret = sock_fd.connect((struct sockaddr*)&conn_addr, addr_len);
    } else if (addr.type() == SockAddr::Type::kLocal) {
        struct sockaddr_un conn_addr;
        socklen_t addr_len = addr.toSockAddr(conn_addr);
        ret = sock_fd.connect((struct sockaddr*)&conn_addr, addr_len);
    } else
        LogErr("not support");

    return ret;
}

void TcpConnector::enterConnectingState()
{
    //! 创建Socket
    SocketFd new_sock_fd = createSocket(server_addr_.type());
    if (new_sock_fd.isNull())
        return;

    LogDbg("server_addr:%s", server_addr_.toString().c_str());

    //! 连接 server_addr_ 指定的地址
    int conn_ret = connect(new_sock_fd, server_addr_);
    int conn_errno = conn_ret == 0 ? 0 : errno;

    //! 检查错误码
    if ((conn_errno == 0) || (conn_errno == EINPROGRESS)
        || (conn_errno == EINTR) || (conn_errno == EISCONN)) {
        //! 正常情况
        sock_fd_ = new_sock_fd;

        CHECK_DELETE_RESET_OBJ(sp_write_ev_);
        sp_write_ev_ = wp_loop_->newFdEvent();
        sp_write_ev_->initialize(sock_fd_, event::FdEvent::kWriteEvent, event::Event::Mode::kOneshot);
        sp_write_ev_->setCallback(std::bind(&TcpConnector::onSocketWritable, this));
        sp_write_ev_->enable();

        state_ = State::kConnecting;
        LogDbg("enter connecting state");

    } else if ((conn_errno == EAGAIN)
        || (conn_errno == EADDRINUSE) || (conn_errno == EADDRNOTAVAIL)
        || (conn_errno == ECONNREFUSED) || (conn_errno == ENETUNREACH)
        || (conn_errno == ENOENT)) {
        LogWarn("connent fail, errno:%d, %s", conn_errno, strerror(conn_errno));
        //! 条件暂时不具备
        onConnectFail();

    } else {
        //! 如果参数不正确
        LogErr("params errno:%d, %s", conn_errno, strerror(conn_errno));
        state_ = State::kNone;
        //!TODO: 如果真失败了该怎么办呢？
    }
}

}
}
