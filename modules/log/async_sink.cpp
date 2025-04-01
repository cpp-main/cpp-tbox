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
#include "async_sink.h"

#include <cstring>
#include <algorithm>
#include <iostream>
#include <chrono>

namespace tbox {
namespace log {

void AsyncSink::cleanup()
{
    if (is_pipe_inited_)
        async_pipe_.cleanup();
}

void AsyncSink::onEnable()
{
    if (!async_pipe_.initialize(cfg_))
        return;

    using namespace std::placeholders;
    async_pipe_.setCallback(std::bind(&AsyncSink::onLogBackEndReadPipe, this, _1, _2));
    is_pipe_inited_ = true;
}

void AsyncSink::onDisable()
{
    async_pipe_.cleanup();
    is_pipe_inited_ = false;
}

void AsyncSink::onLogFrontEnd(const LogContent *content)
{
    async_pipe_.append(content, sizeof(LogContent));
    if (content->text_len != 0)
        async_pipe_.append(content->text_ptr, content->text_len);
}

void AsyncSink::onLogBackEndReadPipe(const void *data_ptr, size_t data_size)
{
    auto start_ts = std::chrono::steady_clock::now();

    buffer_.append(data_ptr, data_size);

    bool is_need_flush = false;
    while (buffer_.readableSize() >= sizeof(LogContent)) {
        LogContent content;
        ::memcpy(&content, buffer_.readableBegin(), sizeof(content));

        auto frame_size = sizeof(LogContent) + content.text_len;
        if (frame_size > buffer_.readableSize())  //! 总结长度不够
            break;

        buffer_.hasRead(sizeof(content));

        content.text_ptr = reinterpret_cast<const char*>(buffer_.readableBegin());
        onLogBackEnd(content);

        is_need_flush = true;
        buffer_.hasRead(content.text_len);
    }

    if (is_need_flush)
        flush();

    auto time_cost = std::chrono::steady_clock::now() - start_ts;
    if (time_cost > std::chrono::milliseconds(500))
        std::cerr << timestamp_str_ << " NOTICE: log sink cost > 500 ms, " << time_cost.count() / 1000 << " us" << std::endl;
}

void AsyncSink::onLogBackEnd(const LogContent &content)
{
    char buff[1024];
    size_t len = 0;

    updateTimestampStr(content.timestamp.sec);

    //! 开启色彩，显示日志等级
    if (enable_color_) {
        len = snprintf(buff, sizeof(buff), "\033[%sm", LOG_LEVEL_COLOR_CODE[content.level]);
        append(buff, len);
    }

    //! 打印等级、时间戳、线程号、模块名
    len = snprintf(buff, sizeof(buff), "%c %s.%06u %ld %s ",
            LOG_LEVEL_LEVEL_CODE[content.level],
            timestamp_str_, content.timestamp.usec,
            content.thread_id, content.module_id);
    append(buff, len);

    if (content.func_name != nullptr) {
        len = snprintf(buff, sizeof(buff), "%s() ", content.func_name);
        append(buff, len);
    }

    if (content.text_len > 0) {
        append(content.text_ptr, content.text_len);
        append(' '); //! 追加空格

        if (content.text_trunc) {
            const char *tip = "(TRUNCATED) ";
            append(tip, ::strlen(tip));
        }
    }

    if (content.file_name != nullptr) {
        len = snprintf(buff, sizeof(buff), "-- %s:%d",  content.file_name, content.line);
        append(buff, len);
    }

    if (enable_color_) {
        append("\033[0m", 4);
    }

    endline();
}

void AsyncSink::append(const char *str, size_t len)
{
    cache_.reserve(cache_.size() + len);
    std::back_insert_iterator<std::vector<char>>  back_insert_iter(cache_);
    std::copy(str, str + len, back_insert_iter);
}

void AsyncSink::append(char ch)
{
    cache_.push_back(ch);
}

}
}
