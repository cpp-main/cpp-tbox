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
 * Copyright (c) 2023 Hevake and contributors, all rights reserved.
 *
 * This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
 * Use of this source code is governed by MIT license that can be found
 * in the LICENSE file in the root of the source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */
#ifndef TBOX_JSONRPC_RPC_H
#define TBOX_JSONRPC_RPC_H

#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <tbox/base/json_fwd.h>
#include <tbox/event/forward.h>
#include <tbox/eventx/timeout_monitor.hpp>

namespace tbox {
namespace jsonrpc {

class Proto;

class Rpc {
  public:
    using RequestCallback = std::function<void(int errcode, const Json &js_result)>;
    using ServiceCallback = std::function<bool(int id, const Json &js_params, int &errcode, Json &js_result)>;

  public:
    explicit Rpc(event::Loop *loop);
    virtual ~Rpc();

    bool initialize(Proto *proto, int timeout_sec = 30);
    void cleanup();

    //! 发送请求或消息，如果cb==nullptr，则是消息
    void request(const std::string &method, const Json &js_params, RequestCallback &&cb);
    void request(const std::string &method, const Json &js_params);
    void request(const std::string &method, RequestCallback &&cb);
    void request(const std::string &method);

    //! 注册当方法被调用时回调什么
    void registeService(const std::string &method, ServiceCallback &&cb);

    //! 发送异步回复
    void respond(int id, int errcode, const Json &js_result);
    void respond(int id, const Json &js_result);
    void respond(int id, int errcode);

  protected:
    void onRecvRequest(int id, const std::string &method, const Json &params);
    void onRecvRespond(int id, int errcode, const Json &result);
    void onRequestTimeout(int id);
    void onRespondTimeout(int id);

  private:
    event::Loop *loop_;
    Proto *proto_ = nullptr;

    std::unordered_map<std::string, ServiceCallback> method_services_;

    int id_alloc_ = 0;
    std::unordered_map<int, RequestCallback> request_callback_;
    std::unordered_set<int> tobe_respond_;
    eventx::TimeoutMonitor<int> request_timeout_;   //! 请求超时监测
    eventx::TimeoutMonitor<int> respond_timeout_;   //! 回复超时监测
};

}
}

#endif //TBOX_JSONRPC_RPC_H
