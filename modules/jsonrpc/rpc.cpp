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
#include "rpc.h"

#include <tbox/base/log.h>
#include <tbox/base/json.hpp>
#include <tbox/base/wrapped_recorder.h>
#include <tbox/util/uuid.h>

#include "proto.h"
#include "inner_types.h"

namespace tbox {
namespace jsonrpc {

Rpc::Rpc(event::Loop *loop, IdType id_type)
    : id_type_(id_type)
    , request_timeout_(loop)
    , respond_timeout_(loop)
{
    using namespace std::placeholders;
    request_timeout_.setCallback(std::bind(&Rpc::onRequestTimeout, this, _1));
    respond_timeout_.setCallback(std::bind(&Rpc::onRespondTimeout, this, _1));

}

Rpc::~Rpc()
{
    respond_timeout_.cleanup();
    request_timeout_.cleanup();
}

bool Rpc::initialize(Proto *proto, int timeout_sec)
{
    using namespace std::placeholders;

    request_timeout_.initialize(std::chrono::seconds(1), timeout_sec);
    respond_timeout_.initialize(std::chrono::seconds(1), timeout_sec);

    if (id_type_ == IdType::kInt) {
        proto->setRecvCallback(
            std::bind(&Rpc::onRecvRequestInt, this, _1, _2, _3),
            std::bind(&Rpc::onRecvRespondInt, this, _1, _2)
        );
    } else {
        str_id_gen_func_ = [] { return util::GenUUID(); };
        proto->setRecvCallback(
            std::bind(&Rpc::onRecvRequestStr, this, _1, _2, _3),
            std::bind(&Rpc::onRecvRespondStr, this, _1, _2)
        );
    }

    proto_ = proto;

    return true;
}

void Rpc::cleanup()
{
    respond_timeout_.cleanup();
    request_timeout_.cleanup();

    tobe_respond_.clear();
    request_callback_.clear();
    str_id_gen_func_ = nullptr;

    method_services_.clear();

    proto_->cleanup();
    proto_ = nullptr;
}

void Rpc::addService(const std::string &method, ServiceCallback &&cb)
{
    method_services_[method] = std::move(cb);
}

void Rpc::removeService(const std::string &method)
{
    method_services_.erase(method);
}

void Rpc::request(const std::string &method, const Json &js_params, RequestCallback &&cb)
{
    RECORD_SCOPE();

    int int_id = 0;

    if (cb) {
        int_id = allocIntId();
        request_callback_[int_id] = std::move(cb);
        request_timeout_.add(int_id);
    }

    if (id_type_ == IdType::kInt) {
        proto_->sendRequest(int_id, method, js_params);

    } else {
        std::string str_id;
        if (int_id != 0) {
            str_id = str_id_gen_func_();

            int_to_str_map_[int_id] = str_id;
            str_to_int_map_[str_id] = int_id;
        }
        proto_->sendRequest(str_id, method, js_params);
    }
}

void Rpc::request(const std::string &method, RequestCallback &&cb)
{
    request(method, Json(), std::move(cb));
}

void Rpc::notify(const std::string &method, const Json &js_params)
{
    request(method, js_params, nullptr);
}

void Rpc::notify(const std::string &method)
{
    request(method, Json(), nullptr);
}

void Rpc::respondResult(int int_id, const Json &js_result)
{
    RECORD_SCOPE();
    if (int_id == 0) {
        LogWarn("send int_id == 0 respond");
        return;
    }

    if (id_type_ == IdType::kInt) {
        proto_->sendResult(int_id, js_result);

    } else {
        auto iter = int_to_str_map_.find(int_id);
        if (iter != int_to_str_map_.end()) {
            std::string str_id = iter->second;
            proto_->sendResult(str_id, js_result);
            int_to_str_map_.erase(iter);
            str_to_int_map_.erase(str_id);
        }
    }

    tobe_respond_.erase(int_id);
}

void Rpc::respondError(int int_id, int errcode, const std::string &message)
{
    RECORD_SCOPE();
    if (int_id == 0) {
        LogWarn("send int_id == 0 respond");
        return;
    }

    if (id_type_ == IdType::kInt) {
        proto_->sendError(int_id, errcode, message);

    } else {
        auto iter = int_to_str_map_.find(int_id);
        if (iter != int_to_str_map_.end()) {
            std::string str_id = iter->second;
            proto_->sendError(str_id, errcode, message);
            int_to_str_map_.erase(iter);
            str_to_int_map_.erase(str_id);
        }
    }

    tobe_respond_.erase(int_id);
}

std::string Rpc::getStrId(int int_id) const
{
    if (id_type_ == IdType::kString) {
        auto iter = int_to_str_map_.find(int_id);
        if (iter != int_to_str_map_.end())
            return iter->second;
    }
    return std::string();
}

void Rpc::setStrIdGenFunc(StrIdGenFunc &&func)
{
    if (id_type_ == IdType::kString && func) {
        str_id_gen_func_ = std::move(func);
    }
}

void Rpc::clear()
{
    int_id_alloc_ = 0;
    request_callback_.clear();
    tobe_respond_.clear();
    request_timeout_.clear();
    respond_timeout_.clear();
    int_to_str_map_.clear();
    str_to_int_map_.clear();
}

void Rpc::onRecvRequestInt(int int_id, const std::string &method, const Json &js_params)
{
    RECORD_SCOPE();
    auto iter = method_services_.find(method);
    if (iter != method_services_.end() && iter->second) {
        Response response;
        if (int_id != 0) {
            tobe_respond_.insert(int_id);
            if (iter->second(int_id, js_params, response)) {
                if (response.error.code == 0) {
                    respondResult(int_id, response.js_result);
                } else {
                    auto &error = response.error;
                    respondError(int_id, error.code, error.message);
                }
            } else {
                respond_timeout_.add(int_id);
            }
        } else {
            iter->second(int_id, js_params, response);
        }
    } else {
        respondError(int_id, ErrorCode::kMethodNotFound, "method not found");
    }
}

void Rpc::onRecvRequestStr(const std::string &str_id, const std::string &method, const Json &js_params)
{
    int int_id = 0;

    if (!str_id.empty()) {
        int_id = allocIntId();
        int_to_str_map_[int_id] = str_id;
        str_to_int_map_[str_id] = int_id;
    }

    onRecvRequestInt(int_id, method, js_params);
}


void Rpc::onRecvRespondInt(int int_id, const Response &response)
{
    RECORD_SCOPE();
    auto iter = request_callback_.find(int_id);
    if (iter != request_callback_.end()) {
        if (iter->second)
            iter->second(response);
        request_callback_.erase(iter);
    }
}

void Rpc::onRecvRespondStr(const std::string &str_id, const Response &response)
{
    //! 收到string的回复，要先转换成int
    RECORD_SCOPE();
    auto iter = str_to_int_map_.find(str_id);
    if (iter != str_to_int_map_.end()) {
        int int_id = iter->second;
        onRecvRespondInt(int_id, response);

        //! 最后要将map中的记录删除
        str_to_int_map_.erase(iter);
        int_to_str_map_.erase(int_id);
    }
}

void Rpc::onRequestTimeout(int int_id)
{
    Response response;
    response.error.code = ErrorCode::kRequestTimeout;
    response.error.message = "request timeout";

    auto iter = request_callback_.find(int_id);
    if (iter != request_callback_.end()) {
        if (iter->second)
            iter->second(response);
        request_callback_.erase(iter);

        //! 如果是string类的，还要将map中的记录删除
        if (id_type_ == IdType::kString) {
            auto iter = int_to_str_map_.find(int_id);
            if (iter != int_to_str_map_.end()) {
                std::string str_id = iter->second;
                int_to_str_map_.erase(iter);
                str_to_int_map_.erase(str_id);
            }
        }
    }
}

void Rpc::onRespondTimeout(int int_id)
{
    auto iter = tobe_respond_.find(int_id);
    if (iter != tobe_respond_.end()) {
        tobe_respond_.erase(iter);

        if (id_type_ == IdType::kInt) {
            LogWarn("respond timeout, int_id:%d", int_id);

        } else {
            //! 如果是string类的，还要将map中的记录删除
            auto iter = int_to_str_map_.find(int_id);
            if (iter != int_to_str_map_.end()) {
                std::string str_id = iter->second;
                LogWarn("respond timeout, int_id:%d, str_id:%s", int_id, str_id.c_str());
                int_to_str_map_.erase(iter);
                str_to_int_map_.erase(str_id);
            }
        }
    }
}

int Rpc::allocIntId()
{
    ++int_id_alloc_;

    //! 防止整数溢出
    if (int_id_alloc_ <= 0)
        int_id_alloc_ = 1;

    return int_id_alloc_;

}

}
}
