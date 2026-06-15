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
 * Copyright (c) 2018 Hevake and contributors, all rights reserved.
 *
 * This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
 * Use of this source code is governed by MIT license that can be found
 * in the LICENSE file in the root of the source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */
#ifndef TBOX_HTTP_RESPOND_H_20220501
#define TBOX_HTTP_RESPOND_H_20220501

#include "common.h"

#include <functional>

namespace tbox {
namespace network {
class TcpConnection;
}
}

namespace tbox {
namespace http {

//! 回复
struct Respond {
    HttpVer http_ver = HttpVer::kUnset;
    StatusCode status_code = StatusCode::kUnset;
    Headers headers;
    std::string body;

    //! 协议升级回调（用于 WebSocket、SSE 等场景）
    //! 中间件检测到升级请求后，设置适当的响应头，并将接管连接的回调注册于此
    //! HTTP 服务器发送响应后，通过此回调将 TcpConnection 交给升级协议处理
    using UpgradeCallback = std::function<void(network::TcpConnection*)>;
    UpgradeCallback upgrade_cb;

    bool isValid() const;
    std::string toString() const;
};

}
}

#endif //TBOX_HTTP_RESPOND_H_20220501
