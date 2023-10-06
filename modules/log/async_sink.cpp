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

constexpr uint32_t LOG_MAX_LEN = (100 << 10);   //! 限定单条日志最大长度

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
    constexpr auto LogContentSize = sizeof(LogContent);
    const char *p = reinterpret_cast<const char*>(data_ptr);

    buffer_.reserve(buffer_.size() + data_size);
    std::back_insert_iterator<std::vector<char>>  back_insert_iter(buffer_);
    std::copy(p, p + data_size, back_insert_iter);

    bool is_need_flush = false;
    while (buffer_.size() >= LogContentSize) {
        auto content = reinterpret_cast<LogContent*>(buffer_.data());
        auto frame_size = LogContentSize + content->text_len;
        if (frame_size > buffer_.size())    //! 总结长度不够
            break;
        content->text_ptr = reinterpret_cast<const char *>(content + 1);
        onLogBackEnd(content);
        is_need_flush = true;
        buffer_.erase(buffer_.begin(), (buffer_.begin() + frame_size));
    }

    if (is_need_flush) {
        flushLog();
        if (buffer_.capacity() > 1024)
            buffer_.shrink_to_fit();
    }
}

void AsyncSink::onLogBackEnd(const LogContent *content)
{
    size_t buff_size = 1024;    //! 初始大小，可应对绝大数情况

    udpateTimestampStr(content->timestamp.sec);

    //! 加循环为了应对缓冲不够的情况
    for (;;) {
        char buff[buff_size];
        size_t pos = 0;

#define REMAIN_SIZE ((buff_size > pos) ? (buff_size - pos) : 0)
#define WRITE_PTR   (buff + pos)

        size_t len = 0;

        //! 开启色彩，显示日志等级
        if (enable_color_) {
            len = snprintf(WRITE_PTR, REMAIN_SIZE, "\033[%dm", LOG_LEVEL_COLOR_NUM[content->level]);
            pos += len;
        }

        //! 打印等级、时间戳、线程号、模块名
        len = snprintf(WRITE_PTR, REMAIN_SIZE, "%c %s.%06u %ld %s ",
                       LOG_LEVEL_COLOR_CODE[content->level],
                       timestamp_str_, content->timestamp.usec,
                       content->thread_id, content->module_id);
        pos += len;

        if (content->func_name != nullptr) {
            len = snprintf(WRITE_PTR, REMAIN_SIZE, "%s() ", content->func_name);
            pos += len;
        }

        if (content->text_len > 0) {
            if (REMAIN_SIZE >= content->text_len)
                memcpy(WRITE_PTR, content->text_ptr, content->text_len);
            pos += content->text_len;

            if (REMAIN_SIZE >= 1)    //! 追加一个空格
                *WRITE_PTR = ' ';
            ++pos;
        }

        if (content->file_name != nullptr) {
            len = snprintf(WRITE_PTR, REMAIN_SIZE, "-- %s:%d", content->file_name, content->line);
            pos += len;
        }

        if (enable_color_) {
            if (REMAIN_SIZE >= 4)
                memcpy(WRITE_PTR, "\033[0m", 4);
            pos += 4;
        }

        if (REMAIN_SIZE >= 2) {
            *WRITE_PTR = '\n';  //! 追加结束符
            ++pos;
            *WRITE_PTR = '\0';  //! 追加结束符
            ++pos;
        } else {
          pos += 2;
        }

#undef REMAIN_SIZE
#undef WRITE_PTR

        //! 如果缓冲区是够用的，就完成
        if (pos <= buff_size) {
            appendLog(buff, pos);
            break;
        }

        //! 否则扩展缓冲区，重来
        buff_size = pos;

        if (buff_size > LOG_MAX_LEN) {
            std::cerr << "WARN: log length " << buff_size << ", too long!" << std::endl;
            break;
        }
    }
}

}
}
