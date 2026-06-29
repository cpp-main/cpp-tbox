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
#include <gtest/gtest.h>
#include "sse_event.h"

namespace tbox {
namespace http {
namespace sse {

//! === SseEvent 构造测试 ===

TEST(SseEvent, DefaultValues)
{
    SseEvent evt;
    EXPECT_EQ(evt.id, "");
    EXPECT_EQ(evt.event, "");
    EXPECT_EQ(evt.data, "");
    EXPECT_EQ(evt.retry, 0);
}

//! === SseEvent::toString() 格式化测试 ===

TEST(SseEvent, ToStringSimpleData)
{
    //! 简单数据：只输出 data 字段
    SseEvent evt;
    evt.data = "hello world";

    //! 预期输出："data: hello world\n\n"
    EXPECT_EQ(evt.toString(), "data: hello world\n\n");
}

TEST(SseEvent, ToStringWithId)
{
    //! 带 id 的数据
    SseEvent evt;
    evt.id = "42";
    evt.data = "hello";

    //! 预期输出："id: 42\ndata: hello\n\n"
    EXPECT_EQ(evt.toString(), "id: 42\ndata: hello\n\n");
}

TEST(SseEvent, ToStringWithEvent)
{
    //! 带 event 类型（非默认 "message"）
    SseEvent evt;
    evt.event = "update";
    evt.data = "status ok";

    //! 预期输出："event: update\ndata: status ok\n\n"
    EXPECT_EQ(evt.toString(), "event: update\ndata: status ok\n\n");
}

TEST(SseEvent, ToStringWithDefaultEvent)
{
    //! event 为 "message"（默认值）时不输出 event 字段
    SseEvent evt;
    evt.event = "message";
    evt.data = "hello";

    //! 预期输出："data: hello\n\n"（不输出 "event: message"）
    EXPECT_EQ(evt.toString(), "data: hello\n\n");
}

TEST(SseEvent, ToStringWithRetry)
{
    //! 带 retry 字段
    SseEvent evt;
    evt.retry = 3000;
    evt.data = "hello";

    //! 预期输出："retry: 3000\ndata: hello\n\n"
    EXPECT_EQ(evt.toString(), "retry: 3000\ndata: hello\n\n");
}

TEST(SseEvent, ToStringRetryZeroNotOutput)
{
    //! retry = 0时不输出 retry 字段
    SseEvent evt;
    evt.retry = 0;
    evt.data = "hello";

    //! 预期输出："data: hello\n\n"（不输出 "retry: 0"）
    EXPECT_EQ(evt.toString(), "data: hello\n\n");
}

TEST(SseEvent, ToStringMultilineData)
{
    //! 多行 data 自动拆分为多个 data: 行
    SseEvent evt;
    evt.data = "line1\nline2\nline3";

    //! 预期输出："data: line1\ndata: line2\ndata: line3\n\n"
    EXPECT_EQ(evt.toString(), "data: line1\ndata: line2\ndata: line3\n\n");
}

TEST(SseEvent, ToStringCompleteEvent)
{
    //! 完整事件：所有字段
    SseEvent evt;
    evt.id = "123";
    evt.event = "update";
    evt.data = "status ok";
    evt.retry = 5000;

    //! 预期输出："retry: 5000\nid: 123\nevent: update\ndata: status ok\n\n"
    EXPECT_EQ(evt.toString(), "retry: 5000\nid: 123\nevent: update\ndata: status ok\n\n");
}

TEST(SseEvent, ToStringEmptyData)
{
    //! data 为空时输出空 data 行
    SseEvent evt;
    evt.id = "1";

    //! 预期输出："id: 1\ndata:\n\n"
    EXPECT_EQ(evt.toString(), "id: 1\ndata:\n\n");
}

}
}
}
