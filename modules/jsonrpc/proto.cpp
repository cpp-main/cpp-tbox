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
#include <tbox/base/log.h>
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

void Proto::sendRequest(const std::string &id, const std::string &method, const Json &js_params)
{
    Json js = {
        {"jsonrpc", "2.0"},
        {"method", method}
    };

    if (!id.empty())
        js["id"] = id;

    if (!js_params.is_null())
        js["params"] = js_params;

    sendJson(js);
}

void Proto::sendRequest(int id, const std::string &method)
{
    sendRequest(id, method, Json());
}

void Proto::sendRequest(const std::string &id, const std::string &method)
{
    sendRequest(id, method, Json());
}

void Proto::sendResult(int id, const Json &js_result)
{
    Json js = {
        {"jsonrpc", "2.0"},
        {"result", js_result}
    };

    if (id != 0)
        js["id"] = id;

    sendJson(js);
}

void Proto::sendResult(const std::string &id, const Json &js_result)
{
    Json js = {
        {"jsonrpc", "2.0"},
        {"result", js_result}
    };

    if (!id.empty())
        js["id"] = id;

    sendJson(js);
}

void Proto::sendError(int id, int errcode, const std::string &message)
{
    Json js = {
        {"jsonrpc", "2.0"},
        {"error", { {"code", errcode } } }
    };

    if (id != 0)
        js["id"] = id;

    if (!message.empty())
        js["error"]["message"] = message;

    sendJson(js);
}

void Proto::sendError(const std::string &id, int errcode, const std::string &message)
{
    Json js = {
        {"jsonrpc", "2.0"},
        {"error", { {"code", errcode } } }
    };

    if (!id.empty())
        js["id"] = id;

    if (!message.empty())
        js["error"]["message"] = message;

    sendJson(js);
}

void Proto::setRecvCallback(RecvRequestIntCallback &&req_cb, RecvRespondIntCallback &&rsp_cb)
{
    if (req_cb && rsp_cb) {
        id_type_ = IdType::kInt;
        recv_request_int_cb_ = std::move(req_cb);
        recv_respond_int_cb_ = std::move(rsp_cb);

    } else {
        LogWarn("int req_cb or rsp_cb is nullptr");
    }
}

void Proto::setRecvCallback(RecvRequestStrCallback &&req_cb, RecvRespondStrCallback &&rsp_cb)
{
    if (req_cb && rsp_cb) {
        id_type_ = IdType::kString;
        recv_request_str_cb_ = std::move(req_cb);
        recv_respond_str_cb_ = std::move(rsp_cb);

    } else {
        LogWarn("str req_cb or rsp_cb is nullptr");
    }
}

void Proto::setSendCallback(SendDataCallback &&cb)
{
    send_data_cb_ = std::move(cb);
}

void Proto::cleanup()
{
    recv_request_int_cb_ = nullptr;
    recv_respond_int_cb_ = nullptr;
    recv_request_str_cb_ = nullptr;
    recv_respond_str_cb_ = nullptr;
    send_data_cb_ = nullptr;
}

void Proto::onRecvJson(const Json &js) const
{
    if (js.is_object()) {

        std::string version;
        if (!util::json::GetField(js, "jsonrpc", version) || version != "2.0") {
            LogNotice("no jsonrpc field, or version not match");
            return;
        }

        if (js.contains("method")) {
            handleAsMethod(js);     //! 按请求进行处理
        } else if (js.contains("result")) {
            handleAsResult(js);     //! 按结果回复进行处理，必须要有id字段
        } else if (js.contains("error")) {
            handleAsError(js);      //! 按错误回复进行处理
        } else {
            LogNotice("not jsonrpc format");
        }

    } else if (js.is_array()) {
        for (auto &js_item : js) {
            onRecvJson(js_item);
        }
    }
}

void Proto::handleAsMethod(const Json &js) const
{
    std::string method;
    if (!util::json::GetField(js, "method", method)) {
        LogNotice("method type not string");
        return;
    }

    auto &js_params = js.contains("params") ? js["params"] : Json();

    if (id_type_ == IdType::kInt) {
        int id = 0;
        if (js.contains("id") && !util::json::GetField(js, "id", id)) {
            LogNotice("id type not int");
            return;
        }
        recv_request_int_cb_(id, method, js_params);

    } else if (id_type_ == IdType::kString) {
        std::string id;
        if (js.contains("id") && !util::json::GetField(js, "id", id)) {
            LogNotice("id type not string");
            return;
        }
        recv_request_str_cb_(id, method, js_params);

    } else {
        LogWarn("please invoke setRecvCallback() first.");
    }
}

void Proto::handleAsResult(const Json &js) const
{
    if (!js.contains("id")) {
        LogNotice("no id in respond");
        return;
    }

    Response response;
    response.js_result = std::move(js["result"]);

    if (id_type_ == IdType::kInt) {
        int id = 0;
        if (!util::json::GetField(js, "id", id)) {
            LogNotice("id type not int in respond");
            return;
        }
        recv_respond_int_cb_(id, response);

    } else if (id_type_ == IdType::kString) {
        std::string id;
        if (!util::json::GetField(js, "id", id)) {
            LogNotice("id type not string in respond");
            return;
        }
        recv_respond_str_cb_(id, response);

    } else {
        LogWarn("please invoke setRecvCallback() first.");
    }
}

void Proto::handleAsError(const Json &js) const
{
    Response response;

    auto &js_error = js["error"];
    if (!util::json::GetField(js_error, "code", response.error.code)) {
        LogNotice("no code field in error");
        return;
    }
    util::json::GetField(js_error, "message", response.error.message);

    if (id_type_ == IdType::kInt) {
        int id = 0;
        if (js.contains("id") && !util::json::GetField(js, "id", id)) {
            LogNotice("id type not int in error");
            return;
        }
        recv_respond_int_cb_(id, response);

    } else if (id_type_ == IdType::kString) {
        std::string id;
        if (js.contains("id") && !util::json::GetField(js, "id", id)) {
            LogNotice("id type not string in error");
            return;
        }
        recv_respond_str_cb_(id, response);

    } else {
        LogWarn("please invoke setRecvCallback() first.");
    }
}

}
}
