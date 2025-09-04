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
#ifndef TBOX_JSONRPC_PROTO_H_20230812
#define TBOX_JSONRPC_PROTO_H_20230812

#include <functional>
#include <tbox/base/json_fwd.h>

namespace tbox {
namespace jsonrpc {

class Proto {
  public:
    using RecvRequestIntCallback = std::function<void(int id, const std::string &method, const Json &params)>;
    using RecvRespondIntCallback = std::function<void(int id, int errcode, const Json &result)>;
    using RecvRequestStrCallback = std::function<void(const std::string &id, const std::string &method, const Json &params)>;
    using RecvRespondStrCallback = std::function<void(const std::string &id, int errcode, const Json &result)>;
    using SendDataCallback = std::function<void(const void* data_ptr, size_t data_size)>;

    void setRecvCallback(RecvRequestIntCallback &&req_cb, RecvRespondIntCallback &&rsp_cb);
    void setRecvCallback(RecvRequestStrCallback &&req_cb, RecvRespondStrCallback &&rsp_cb);
    void setSendCallback(SendDataCallback &&cb);
    void cleanup();

    void setLogEnable(bool is_enable) { is_log_enabled_ = is_enable; }
    void setLogLabel(const std::string &log_label) { log_label_ = log_label; }

  public:
    void sendRequest(int id, const std::string &method);
    void sendRequest(int id, const std::string &method, const Json &js_params);
    void sendResult(int id, const Json &js_result);
    void sendError(int id, int errcode, const std::string &message = "");

    void sendRequest(const std::string &id, const std::string &method);
    void sendRequest(const std::string &id, const std::string &method, const Json &js_params);
    void sendResult(const std::string &id, const Json &js_result);
    void sendError(const std::string &id, int errcode, const std::string &message = "");

  public:
    /**
     * 当传输层收到数据后调用。该方法进行解包然后进行后续的处理
     */
    virtual ssize_t onRecvData(const void *data_ptr, size_t data_size) = 0;

  protected:
    enum class IdType { kNone, kInt, kString };

    virtual void sendJson(const Json &js) = 0;

    void onRecvJson(const Json &js) const;
    void handleAsMethod(const Json &js) const;
    void handleAsResult(const Json &js) const;
    void handleAsError(const Json &js) const;

    IdType id_type_ = IdType::kNone;
    RecvRequestIntCallback recv_request_int_cb_;
    RecvRespondIntCallback recv_respond_int_cb_;
    RecvRequestStrCallback recv_request_str_cb_;
    RecvRespondStrCallback recv_respond_str_cb_;

    SendDataCallback    send_data_cb_;

    bool is_log_enabled_ = false;
    std::string log_label_;
};

}
}

#endif //TBOX_JSONRPC_PROTO_H_20230812
