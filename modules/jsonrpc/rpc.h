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
    /**
     * 收到对端回复的回调函数
     *
     * \param   errcode     错误码，= 0 表示没有错误
     * \param   js_result   回复的结果
     */
    using RequestCallback = std::function<void(int errcode, const Json &js_result)>;

    /**
     * 收到对端请求时的回调函数
     *
     * \param   id          请求的id（用于异步回复使用）
     * \param   js_params   请求的参数
     * \param   errcode     将要回复的错误码，= 0 表示没有错误（仅同步回复有效）
     * \param   js_result   将要回复的结果，只有在 errcode == 0 时才会有效（仅同步回复有效）
     *
     * \return  true        同步回复：在函数返回后自动回复，根据errcode与js_result进行回复
     * \return  false       异步回复：不在函数返回后自动回复，而是在稍候通过调respond()进行回复
     */
    using ServiceCallback = std::function<bool(int id, const Json &js_params, int &errcode, Json &js_result)>;

  public:
    explicit Rpc(event::Loop *loop);
    virtual ~Rpc();

    bool initialize(Proto *proto, int timeout_sec = 30);
    void cleanup();

    //! 添加方法被调用时的回调函数
    void addService(const std::string &method, ServiceCallback &&cb);

    //! 发送请求（需要回复的）
    void request(const std::string &method, const Json &js_params, RequestCallback &&cb);
    void request(const std::string &method, RequestCallback &&cb);

    //! 发送通知（不需要回复的）
    void notify(const std::string &method, const Json &js_params);
    void notify(const std::string &method);

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
