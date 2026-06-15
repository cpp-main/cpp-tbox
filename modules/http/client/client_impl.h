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
#ifndef TBOX_HTTP_CLIENT_IMP_H_20260614
#define TBOX_HTTP_CLIENT_IMP_H_20260614

#include <deque>
#include <chrono>
#include <limits>

#include <tbox/network/tcp_client.h>
#include <tbox/base/defines.h>

#include "client.h"
#include "respond_parser.h"

namespace tbox {
namespace http {

struct Request;

namespace client {

using namespace event;
using namespace network;
using namespace std;

class Client::Impl {
  public:
    Impl(Client *wp_parent, event::Loop *wp_loop);
    ~Impl();

  public:
    bool initialize(const SockAddr &server_addr);
    bool start();
    void stop();
    void cleanup();
    State state() const { return state_; }

    void request(const Request &req, const RespondCallback &cb);
    void request(Method method, const string &path, const RespondCallback &cb);
    void request(Method method, const string &path, const string &body,
                  const Headers &headers, const RespondCallback &cb);

    void setAutoReconnect(bool enable);
    void setReconnectDelayCalcFunc(const ReconnectDelayCalc &func);
    void setRequestTimeout(chrono::milliseconds ms);
    void setContextLogEnable(bool enable);

    void setConnectedCallback(const ConnectedCallback &cb);
    void setConnectFailCallback(const ConnectFailCallback &cb);
    void setDisconnectedCallback(const DisconnectedCallback &cb);

  private:
    void onTcpConnected();
    void onTcpConnectFail();
    void onTcpDisconnected();
    void onTcpReceived(Buffer &buff);

    //! 发送一个请求
    void sendRequest(const Request &req, const RespondCallback &cb);

    //! 对所有 pending 请求执行错误回调（断线或超时）
    void failAllPendingRequests(StatusCode status_code, const string &message);

    //! 发送缓存中的所有请求
    void sendCachedRequests();

    //! 请求超时处理
    void onRequestTimeout(int req_id);

  private:
    Client *wp_parent_;
    event::Loop *wp_loop_;

    network::TcpClient tcp_client_;
    RespondParser res_parser_;

    //! 请求队列
    struct PendingRequest {
        int req_id;
        RespondCallback cb;
        event::TimerEvent *sp_timer = nullptr;  //! 超时定时器
    };
    deque<PendingRequest> pending_requests_;
    int next_req_id_ = 0;
    chrono::milliseconds request_timeout_ms_ = chrono::seconds(30);

    //! 在未连接时缓存的请求
    struct CachedRequest {
        Request *sp_req;
        RespondCallback cb;
    };
    deque<CachedRequest> cached_requests_;

    State state_ = State::kNone;
    bool auto_reconnect_ = true;
    bool context_log_enable_ = false;

    ConnectedCallback connected_cb_;
    ConnectFailCallback connect_fail_cb_;
    DisconnectedCallback disconnected_cb_;

    int cb_level_ = 0;
};

}
}
}
#endif //TBOX_HTTP_CLIENT_IMP_H_20260614
