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
#ifndef TBOX_NETWORK_TCP_CONNECTOR_H_20180115
#define TBOX_NETWORK_TCP_CONNECTOR_H_20180115

#include <tbox/base/defines.h>
#include <tbox/event/loop.h>
#include <tbox/event/fd_event.h>
#include <tbox/event/timer_event.h>

#include "sockaddr.h"
#include "socket_fd.h"

namespace tbox {
namespace network {

class TcpConnection;

class TcpConnector {
  public:
    explicit TcpConnector(event::Loop *wp_loop);
    virtual ~TcpConnector();

    NONCOPYABLE(TcpConnector);
    IMMOVABLE(TcpConnector);

    //! 状态
    enum class State {
        kNone,              //!< 未初始化
        kInited,            //!< 空闲未连接
        kReconnectDelay,    //!< 重连等待
        kConnecting,        //!< 连接中
    };

    //! 函数类型定义
    using ConnectedCallback     = std::function<void(TcpConnection*)>;
    using ConnectFailCallback   = std::function<void()>;
    using ReconnectDelayCalc    = std::function<int(int)>;

    //! 必需设置项
    void initialize(const SockAddr &server_addr);           //!< 设置服务端地址
    void setConnectedCallback(const ConnectedCallback &cb); //!< 设置连接成功的回调

    //! 非必需设置项
    void setTryTimes(int try_times);    //!< 设置连接尝试次数
    void setConnectFailCallback(const ConnectFailCallback &cb);     //!< 设置连接失败的回调
    void setReconnectDelayCalcFunc(const ReconnectDelayCalc &func); //!< 设置用户自定义的重连延迟策略

    bool start();   //!< 开始连接
    void stop();    //!< 停止连接

    void cleanup(); //!< 清理

    State state() const { return state_; }

  protected:
    virtual SocketFd createSocket(SockAddr::Type addr_type) const;
    virtual int connect(SocketFd sock_fd, const SockAddr &addr) const;

    void checkSettingAndTryEnterIdleState();

    void enterConnectingState();        //!< 进入连接状态的操作
    void exitConnectingState();         //!< 退出连接状态的操作
    void enterReconnectDelayState();    //!< 进入重连等待状态的操作
    void exitReconnectDelayState();     //!< 退出重连等待状态的操作

    void onConnectFail();
    void onSocketWritable();    //!< 当连接成功时的处理
    void onDelayTimeout();      //!< 当等待延时到期后的处理

  private:
    event::Loop *wp_loop_ = nullptr;

    State state_ = State::kNone;    //! 当前状态

    //! 设置项
    SockAddr server_addr_;  //!< 目标服务器地址
    ConnectedCallback       connected_cb_;
    ConnectFailCallback     connect_fail_cb_;
    ReconnectDelayCalc      reconn_delay_calc_func_;
    int try_times_ = 0;     //!< 尝试连接次数，默认一直尝试

    SocketFd sock_fd_;

    event::FdEvent    *sp_write_ev_ = nullptr;   //!< connect 过程的结果监听事件
    event::TimerEvent *sp_delay_ev_ = nullptr;   //!< 重连等待延时定时器

    int conn_fail_times_ = 0;   //! 连续 connect 失败计数
    int cb_level_ = 0;
};

}
}

#endif //TBOX_NETWORK_TCP_CONNECTOR_H_20180115
