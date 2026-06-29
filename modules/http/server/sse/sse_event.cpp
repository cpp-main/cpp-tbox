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
 * Copyright (c) 2026 Hevake and contributors, all rights reserved.
 *
 * This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
 * Use of this source code is governed by MIT license that can be found
 * in the LICENSE file in the root of the source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */
#include "sse_event.h"

#include <sstream>

namespace tbox {
namespace http {
namespace sse {

std::string SseEvent::toString() const
{
    //! SSE 协议格式（W3C/WHATWG EventSource 规范）：
    //! 每个字段以 "field: value\n" 格式输出
    //! 事件以空行 "\n" 结束（标志事件完成）
    //!
    //! 字段输出顺序：retry → id → event → data → 空行
    //! 顺序不影响浏览器解析，但统一顺序便于调试

    std::string result;

    //! retry 字段（可选，仅在 retry > 0 时输出）
    //! 告知浏览器断线后多久自动重连
    if (retry > 0)
        result += "retry: " + std::to_string(retry) + "\n";

    //! id 字段（可选）
    //! 浏览器重连时通过 Last-Event-ID 头部携带此值
    if (!id.empty())
        result += "id: " + id + "\n";

    //! event 字段（可选）
    //! 默认为 "message"，浏览器通过 .onmessage 监听
    //! 自定义 event 类型通过 .addEventListener(event, ...) 监听
    //! 不输出默认值 "message"，减少传输量
    if (!event.empty() && event != "message")
        result += "event: " + event + "\n";

    //! data 字段（必须）
    //! 多行 data 自动拆分为多个 `data:` 行
    //! 例如 data="line1\nline2" 输出为 "data: line1\ndata: line2\n"
    if (!data.empty()) {
        std::istringstream iss(data);
        std::string line;
        while (std::getline(iss, line))
            result += "data: " + line + "\n";
    } else {
        //! data 为空时仍需输出空 data 行（保持协议完整性）
        result += "data:\n";
    }

    //! 事件结束标志（空行）
    //! 浏览器 EventSource 在收到空行时认为事件完成并触发回调
    result += "\n";

    return result;
}

}
}
}
