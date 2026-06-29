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
#include "respond.h"
#include <sstream>

namespace tbox {
namespace http {

bool Respond::isValid() const
{
    return status_code != StatusCode::kUnset && http_ver != HttpVer::kUnset;
}

std::string Respond::toString() const
{
    std::ostringstream oss;
    oss << HttpVerToString(http_ver) << " " << StatusCodeToString(status_code) << CRLF;

    bool has_content_length = false;
    for (auto &head : headers) {
        oss << head.first << ": " << head.second << CRLF;
        if (head.first == "Content-Length")
            has_content_length = true;
    }

    //! 当 upgrade_cb 已设置时（WebSocket 101、SSE 200 等），不自动添加 Content-Length
    //! 原因：升级/流式响应后面是持续的数据流（WebSocket 帧、SSE 事件），不是定长 body
    //! Content-Length 会误导浏览器认为响应已完成，阻止流式数据接收
    if (!has_content_length && !upgrade_cb)
        oss << "Content-Length: " << body.length() << CRLF;

    oss << CRLF;
    oss << body;

    return oss.str();
}

}
}

