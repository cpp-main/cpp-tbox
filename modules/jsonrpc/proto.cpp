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
#include "proto.h"
#include <tbox/base/json.hpp>
#include <tbox/base/assert.h>
#include <tbox/util/json.h>

namespace tbox {
namespace jsonrpc {

void Proto::sendRequest(int id, const std::string &method, const Json &js_params)
{
    Json js = {
        {"jsonrpc", "2.0"},
        {"method", method}
    };

    if (id != 0)
        js["id"] = id;

    if (!js_params.is_null())
        js["params"] = js_params;

    sendJson(js);
}

void Proto::sendRequest(int id, const std::string &method)
{
    sendRequest(id, method, Json());
}

void Proto::sendResult(int id, const Json &js_result)
{
    Json js = {
        {"jsonrpc", "2.0"},
        {"id", id},
        {"result", js_result}
    };

    sendJson(js);
}

void Proto::sendError(int id, int errcode, const std::string &message)
{
    Json js = {
        {"jsonrpc", "2.0"},
        {"id", id},
        {"error", { {"code", errcode } } }
    };

    if (!message.empty())
        js["error"]["message"] = message;

    sendJson(js);
}

void Proto::setRecvCallback(RecvRequestCallback &&req_cb, RecvRespondCallback &&rsp_cb)
{
    recv_request_cb_ = std::move(req_cb);
    recv_respond_cb_ = std::move(rsp_cb);
}

void Proto::setSendCallback(SendDataCallback &&cb)
{
    send_data_cb_ = std::move(cb);
}

void Proto::onRecvJson(const Json &js)
{
    if (js.is_object()) {

        std::string version;
        if (!util::json::GetField(js, "jsonrpc", version) || version != "2.0") {
            LogNotice("no jsonrpc field, or version not match");
            return;
        }

        if (js.contains("method")) {
            if (!recv_request_cb_)
                return;

            //! 按请求进行处理
            std::string method;
            int id = 0;
            if (!util::json::GetField(js, "method", method)) {
                LogNotice("method type not string");
                return;
            }
            util::json::GetField(js, "id", id);
            recv_request_cb_(id, method, js.contains("params") ? js["params"] : Json());

        } else if (js.contains("result")) {
            //! 按结果回复进行处理
            if (!recv_respond_cb_)
                return;

            int id = 0; 
            if (!util::json::GetField(js, "id", id)) {
                LogNotice("no method field in respond");
                return;
            }
            recv_respond_cb_(id, 0, js["result"]);

        } else if (js.contains("error")) {
            //! 按错误回复进行处理
            if (!recv_respond_cb_)
                return;

            int id = 0; 
            util::json::GetField(js, "id", id);

            auto &js_error = js["error"];
            int errcode = 0;
            if (!util::json::GetField(js_error, "code", errcode)) {
                LogNotice("no code field in error");
                return;
            }

            recv_respond_cb_(id, errcode, Json());

        } else {
            LogNotice("not jsonrpc format");
        }

    } else if (js.is_array()) {
        for (auto &js_item : js) {
            onRecvJson(js_item);
        }
    }
}

}
}
