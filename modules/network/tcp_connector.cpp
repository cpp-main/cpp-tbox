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
#include "tcp_connector.h"

#include <sys/un.h>

#include <tbox/base/log.h>
#include <tbox/base/assert.h>
#include <tbox/base/wrapped_recorder.h>

#include "tcp_connection.h"

namespace tbox {
namespace network {

TcpConnector::TcpConnector(event::Loop *wp_loop) :
    wp_loop_(wp_loop),
    reconn_delay_calc_func_([](int) {return 1;})
{ }

TcpConnector::~TcpConnector()
{
    TBOX_ASSERT(cb_level_ == 0);

    cleanup();

    CHECK_DELETE_RESET_OBJ(sp_delay_ev_);
    CHECK_DELETE_RESET_OBJ(sp_write_ev_);
}

void TcpConnector::checkSettingAndTryEnterIdleState()
{
    if (state_ == State::kNone) {
        if (server_addr_.type() != SockAddr::Type::kNone && connected_cb_)
            state_ = State::kInited;
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
    if (state_ != State::kInited) {
        LogWarn("not in idle state");
        return false;
    }

    conn_fail_times_ = 0;
    enterConnectingState();
    return true;
}

void TcpConnector::stop()
{
    if ((state_ == State::kInited) || (state_ == State::kNone))
        return;

    if (state_ == State::kConnecting)
        exitConnectingState();
    else if (state_ == State::kReconnectDelay)
        exitReconnectDelayState();

    state_ = State::kInited;
}

void TcpConnector::cleanup()
{
    if (state_ <= State::kNone)
        return;

    stop();

    connected_cb_ = nullptr;
    connect_fail_cb_ = nullptr;
    reconn_delay_calc_func_ = [](int) {return 1;};
    try_times_ = 0;
    conn_fail_times_ = 0;

    state_ = State::kNone;
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
        sock_fd_ = std::move(new_sock_fd);

        CHECK_DELETE_RESET_OBJ(sp_write_ev_);
        sp_write_ev_ = wp_loop_->newFdEvent("TcpConnector::sp_write_ev_");
        sp_write_ev_->initialize(sock_fd_.get(), event::FdEvent::kWriteEvent, event::Event::Mode::kOneshot);
        sp_write_ev_->setCallback(std::bind(&TcpConnector::onSocketWritable, this));
        sp_write_ev_->enable();

        state_ = State::kConnecting;
        LogDbg("enter connecting state");

    } else if ((conn_errno == EAGAIN)
        || (conn_errno == EADDRINUSE) || (conn_errno == EADDRNOTAVAIL)
        || (conn_errno == ECONNREFUSED) || (conn_errno == ENETUNREACH)
        || (conn_errno == ENOENT)) {
        LogNotice("connent fail, errno:%d, %s", conn_errno, strerror(conn_errno));
        //! 条件暂时不具备
        onConnectFail();

    } else {
        //! 如果参数不正确
        LogErr("params errno:%d, %s", conn_errno, strerror(conn_errno));
        state_ = State::kNone;
        //!TODO: 如果真失败了该怎么办呢？
    }
}

void TcpConnector::exitConnectingState()
{
    sp_write_ev_->disable();

    //! 释放 sp_write_ev_ 对象
    event::FdEvent *tmp = nullptr;
    std::swap(tmp, sp_write_ev_);

    wp_loop_->runNext(
        [tmp] { CHECK_DELETE_OBJ(tmp); },
        "TcpConnector::exitConnectingState, delete tmp"
    );

    //! 关闭 socket
    sock_fd_.close();
    LogDbg("exit connecting state");
}

void TcpConnector::enterReconnectDelayState()
{
    //! 计算出要延时等待的时长
    ++cb_level_;
    int delay_sec = reconn_delay_calc_func_(conn_fail_times_);
    --cb_level_;

    //! 创建定时器进行等待
    CHECK_DELETE_RESET_OBJ(sp_delay_ev_);
    sp_delay_ev_ = wp_loop_->newTimerEvent("TcpConnector::sp_delay_ev_");
    sp_delay_ev_->initialize(std::chrono::seconds(delay_sec), event::Event::Mode::kOneshot);
    sp_delay_ev_->setCallback(std::bind(&TcpConnector::onDelayTimeout, this));
    sp_delay_ev_->enable();

    state_ = State::kReconnectDelay;
    LogDbg("enter reconnect delay state, delay: %d", delay_sec);
}

void TcpConnector::exitReconnectDelayState()
{
    //! 关闭定时器
    sp_delay_ev_->disable();

    //! 销毁定时器
    event::TimerEvent *tmp = nullptr;
    std::swap(tmp, sp_delay_ev_);

    wp_loop_->runNext(
        [tmp] { CHECK_DELETE_OBJ(tmp); },
        "TcpConnector::exitReconnectDelayState, delete tmp"
    );

    LogDbg("exit reconnect delay state");
}

void TcpConnector::onConnectFail()
{
    ++conn_fail_times_;
    //! 如果设置了尝试次数，且超过了尝试次数，则回调 connect_fail_cb_ 然后回到 State::kInited
    //! 否则继续进入重连等待延时状态
    if ((try_times_ > 0) && (conn_fail_times_ >= try_times_)) {
        if (connect_fail_cb_) {
            ++cb_level_;
            connect_fail_cb_();
            --cb_level_;
        } else
            LogNotice("connector stoped");

        state_ = State::kInited;
    } else
        enterReconnectDelayState();
}

void TcpConnector::onSocketWritable()
{
    RECORD_SCOPE();
    //! 读取Socket错误码
    int sock_errno = 0;
    socklen_t optlen = sizeof(sock_errno);
    if (sock_fd_.getSocketOpt(SOL_SOCKET, SO_ERROR, &sock_errno, &optlen)) {
        if (sock_errno == 0) {  //! 连接成功
            SocketFd conn_sock_fd = sock_fd_;
            sock_fd_.reset();

            exitConnectingState();
            state_ = State::kInited;

            LogInfo("connect to %s success", server_addr_.toString().c_str());
            if (connected_cb_) {
                auto sp_conn = new TcpConnection(wp_loop_, conn_sock_fd, server_addr_);
                sp_conn->enable();
                ++cb_level_;
                connected_cb_(sp_conn);
                --cb_level_;
            } else
                LogWarn("connected callback is not set");

        } else {    //! 连接失败
            LogNotice("connect fail, errno:%d, %s", sock_errno, strerror(sock_errno));
            //! 状态切换：kConnecting --> kReconnectDelay
            exitConnectingState();
            onConnectFail();
        }
    } else {    //! 没有读取到 SO_ERROR
        LogErr("getSocketOpt fail, errno:%d, %s", errno, strerror(errno));
    }
}

void TcpConnector::onDelayTimeout()
{
    //! 状态切换：kReconnectDelay --> kConnecting
    exitReconnectDelayState();
    enterConnectingState();
}

}
}
