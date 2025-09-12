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
#ifndef TBOX_JSONRPC_PRC_H
#define TBOX_JSONRPC_PRC_H

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
    //! id 类型：整数、字串
    enum class IdType { kInt, kString };

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
     * \param   int_id      请求的id（用于异步回复使用）
     *                      如果是string类的id，那么可用getStrId()获取真实字串ID
     * \param   js_params   请求的参数
     * \param   errcode     将要回复的错误码，= 0 表示没有错误（仅同步回复有效）
     * \param   js_result   将要回复的结果，只有在 errcode == 0 时才会有效（仅同步回复有效）
     *
     * \return  true        同步回复：在函数返回后自动回复，根据errcode与js_result进行回复
     * \return  false       异步回复：不在函数返回后自动回复，而是在稍候通过调respond()进行回复
     */
    using ServiceCallback = std::function<bool(int int_id, const Json &js_params, int &errcode, Json &js_result)>;

    //! 生成std::string类型ID的函数
    using StrIdGenFunc = std::function<std::string()>;

  public:
    explicit Rpc(event::Loop *loop, IdType id_type = IdType::kInt);
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
    void respond(int int_id, int errcode, const Json &js_result);
    void respondResult(int int_id, const Json &js_result);
    void respondError(int int_id, int errcode);

    //! 仅在IdType::kString时有效的函数

    //! 根据int_id获取实际的str_id
    std::string getStrId(int int_id) const;

    //! 设置string id生成函数，默认为UUIDv4
    void setStrIdGenFunc(StrIdGenFunc &&func);

  protected:
    void onRecvRequestInt(int int_id, const std::string &method, const Json &params);
    void onRecvRespondInt(int int_id, int errcode, const Json &result);
    void onRecvRequestStr(const std::string &str_id, const std::string &method, const Json &params);
    void onRecvRespondStr(const std::string &str_id, int errcode, const Json &result);

    void onRequestTimeout(int int_id);
    void onRespondTimeout(int int_id);

    int  allocIntId();

  private:
    IdType id_type_;
    Proto *proto_ = nullptr;

    std::unordered_map<std::string, ServiceCallback> method_services_;

    int int_id_alloc_ = 0;

    std::unordered_map<int, RequestCallback> request_callback_;
    std::unordered_set<int> tobe_respond_;          //!< 待回复的请求
    eventx::TimeoutMonitor<int> request_timeout_;   //!< 请求超时监测
    eventx::TimeoutMonitor<int> respond_timeout_;   //!< 回复超时监测

    //! 字串ID类的成员变量
    StrIdGenFunc str_id_gen_func_;  //!< 生成字串ID的函数对象
    std::map<int, std::string> int_to_str_map_; //!< int --> string 映射
    std::map<std::string, int> str_to_int_map_; //!< string --> int 映射
};

}
}

#endif //TBOX_JSONRPC_PRC_H
