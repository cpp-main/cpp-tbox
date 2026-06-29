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
#ifndef TBOX_HTTP_SSE_EVENT_H_20260616
#define TBOX_HTTP_SSE_EVENT_H_20260616

#include <string>
#include <cstdint>

namespace tbox {
namespace http {
namespace sse {

//! SSE 事件（Server-Sent Events, W3C/WHATWG 规范）
//! 对应 SSE 协议中的字段：id、event、data、retry
//! toString() 将事件格式化为标准 SSE 文本格式
struct SseEvent {
    //! 事件 ID（可选）
    //! 对应 `id:` 字段，浏览器重连时通过 Last-Event-ID 头部携带此值
    //! 用于实现断线续传：服务端可根据 Last-Event-ID 从断点继续推送
    std::string id;

    //! 事件类型（可选）
    //! 对应 `event:` 字段，默认为 "message"
    //! 浏览器 EventSource 对象通过 .onmessage 或 .addEventListener(event, ...) 监听
    std::string event;

    //! 数据（必须）
    //! 对应 `data:` 字段，支持多行文本
    //! toString() 会自动将多行 data 拆分为多个 `data:` 行
    std::string data;

    //! 重连间隔毫秒数（可选）
    //! 对应 `retry:` 字段，告知浏览器断线后多久自动重连
    //! 仅在 retry > 0 时输出
    int retry = 0;

    //! 将事件格式化为 SSE 文本协议格式
    //! 输出规则（W3C/WHATWG EventSource 规范）：
    //!   - retry > 0 时输出 "retry: <value>\n"
    //!   - id 非空时输出 "id: <value>\n"
    //!   - event 非空且不等于 "message" 时输出 "event: <value>\n"
    //!   - data 按行拆分，每行输出 "data: <line>\n"
    //!   - 最后以空行 "\n" 结束（标志事件完成）
    //!
    //! 示例输出：
    //!   "id: 42\nevent: update\ndata: hello\n\n"
    //!   多行 data：
    //!   "data: line1\ndata: line2\n\n"
    std::string toString() const;
};

}
}
}

#endif //TBOX_HTTP_SSE_EVENT_H_20260616
