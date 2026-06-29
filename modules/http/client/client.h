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
 * Copyright (c) 2026 Hevake and contributors, all rights reserved.
 *
 * This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
 * Use of this source code is governed by MIT license that can be found
 * in the LICENSE file in the root of the source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */
#ifndef TBOX_HTTP_CLIENT_H_20220504
#define TBOX_HTTP_CLIENT_H_20220504

#include <tbox/event/loop.h>
#include <tbox/network/sockaddr.h>
#include <tbox/network/tls_config.h>
#include <tbox/base/defines.h>

#include "../common.h"
#include "../request.h"
#include "../respond.h"

namespace tbox {
namespace http {
namespace client {

class Client {
  public:
    explicit Client(event::Loop *wp_loop);
    virtual ~Client();

    NONCOPYABLE(Client);
    IMMOVABLE(Client);

  public:
    //! 状态
    enum class State {
        kNone,          //!< 未初始化
        kInited,        //!< 已初始化
        kConnecting,    //!< 连接中
        kConnected,     //!< 已连接
        kReconnWaiting, //!< 断连等待重连中
    };

    //! 初始化，设置目标服务器地址
    bool initialize(const network::SockAddr &server_addr);

    //! 设置 TLS 配置（必须在 initialize() 之前调用）
    //! 需要 network_tls 模块支持，未链接时调用无效
    void setTlsConfig(const network::TlsConfig &config);

    bool start();       //!< 开始连接
    void stop();        //!< 停止/断开连接
    void cleanup();     //!< 清理，与 initialize() 是逆操作

    State state() const;

  public:
    //! 收到回复时的回调
    using RespondCallback = std::function<void(const Respond &res)>;

    //! 发送请求（完整 Request 对象）
    void request(const Request &req, const RespondCallback &cb);

    //! 发送请求（便捷方法：指定 Method 和 path）
    void request(Method method, const std::string &path, const RespondCallback &cb);

    //! 发送请求（便捷方法：指定 Method、path、body、headers）
    void request(Method method, const std::string &path,
                 const std::string &body, const Headers &headers,
                 const RespondCallback &cb);

  public:
    //! 连接相关回调
    using ConnectedCallback    = std::function<void()>;
    using ConnectFailCallback  = std::function<void()>;
    using DisconnectedCallback = std::function<void()>;
    using ReconnectDelayCalc   = std::function<int(int)>;

    void setConnectedCallback(const ConnectedCallback &cb);
    void setConnectFailCallback(const ConnectFailCallback &cb);
    void setDisconnectedCallback(const DisconnectedCallback &cb);

    //! 配置
    void setAutoReconnect(bool enable);
    void setReconnectDelayCalcFunc(const ReconnectDelayCalc &func);
    void setRequestTimeout(std::chrono::milliseconds ms);
    void setContextLogEnable(bool enable);

  private:
    class Impl;
    Impl *impl_;
};

}
}
}
#endif //TBOX_HTTP_CLIENT_H_20220504
