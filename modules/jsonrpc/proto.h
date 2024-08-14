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
    using RecvRequestCallback = std::function<void(int id, const std::string &method, const Json &params)>;
    using RecvRespondCallback = std::function<void(int id, int errcode, const Json &result)>;
    using SendDataCallback = std::function<void(const void* data_ptr, size_t data_size)>;

    void setRecvCallback(RecvRequestCallback &&req_cb, RecvRespondCallback &&rsp_cb);
    void setSendCallback(SendDataCallback &&cb);

    void setLogEnable(bool is_enable) { is_log_enabled_ = is_enable; }
    void setLogLabel(const std::string &log_label) { log_label_ = log_label; }

  public:
    void sendRequest(int id, const std::string &method);
    void sendRequest(int id, const std::string &method, const Json &js_params);

    void sendResult(int id, const Json &js_result);
    void sendError(int id, int errcode, const std::string &message = "");

  public:
    /**
     * 当传输层收到数据后调用。该方法进行解包然后进行后续的处理
     */
    virtual ssize_t onRecvData(const void *data_ptr, size_t data_size) = 0;

  protected:
    virtual void sendJson(const Json &js) = 0;

    void onRecvJson(const Json &js);

    RecvRequestCallback recv_request_cb_;
    RecvRespondCallback recv_respond_cb_;
    SendDataCallback    send_data_cb_;

    bool is_log_enabled_ = false;
    std::string log_label_;
};

}
}

#endif //TBOX_JSONRPC_PROTO_H_20230812
