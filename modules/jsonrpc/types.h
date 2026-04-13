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
#include <string>
#include <tbox/base/json.hpp>

#ifndef TBOX_JSONRPC_TYPES_H_20251026
#define TBOX_JSONRPC_TYPES_H_20251026

namespace tbox {
namespace jsonrpc {

//! 错误码
enum ErrorCode {
    kParseError     = -32700,   //!< 解析错误
    kInvalidRequest = -32600,   //!< 不合法的请求
    kMethodNotFound = -32601,   //!< 找不到方法
    kInvalidParams  = -32602,   //!< 参数不合法
    kInternalError  = -32603,   //!< 内部异常
    kRequestTimeout = -32000,   //!< 请求超时
};

//! id 类型：整数、字串
enum class IdType {
    kNone,
    kInt,       //!< 整数id
    kString     //!< 字串uuid
};

//! 回复
struct Response {
    Json js_result; //! 结果

    //! 错误相关
    struct {
        int code = 0;         //! 错误码
        std::string message;  //! 错误描述
    } error;
};

}
}

#endif //TBOX_JSONRPC_TYPES_H_20251026
