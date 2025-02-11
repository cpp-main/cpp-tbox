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
#include "log_impl.h"

#include <sys/time.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <cstring>
#include <vector>
#include <iostream>
#include <algorithm>
#include <mutex>

namespace {

size_t _LogTextMaxLength = (100 << 10);     //! 限定单条日志最大长度，默认为100KB

std::mutex _lock;
uint32_t _id_alloc = 0;

struct OutputChannel {
    uint32_t id;
    LogPrintfFuncType func;
    void *ptr;
};

std::vector<OutputChannel> _output_channels;

const char* Basename(const char *full_path)
{
    const char *p_last = full_path;
    if (p_last != nullptr) {
        for (const char *p = full_path; *p; ++p) {
            if (*p == '/')
                p_last = p + 1;
        }
    }
    return p_last;
}

bool CantDispatch()
{
    std::lock_guard<std::mutex> lg(_lock);
    return _output_channels.empty();
}

void Dispatch(const LogContent &content)
{
    std::lock_guard<std::mutex> lg(_lock);
    for (const auto &item : _output_channels) {
        if (item.func)
            item.func(&content, item.ptr);
    }
}

}

const char  LOG_LEVEL_LEVEL_CODE[LOG_LEVEL_MAX] = {
    'F', 'E', 'W', 'N', 'I', 'I', 'D', 'T'
};

const char* LOG_LEVEL_COLOR_CODE[LOG_LEVEL_MAX] = {
    "7;91",     //! FATAL       文字黑，背景亮红
    "31",       //! ERROR       红
    "7;93",     //! WARN        文字黑，背景亮黄
    "93",       //! NOTICE      亮黄
    "7;92",     //! IMPORTANT   文字黑，背景亮绿
    "32",       //! INFO        绿
    "36",       //! DEBUG       青
    "35",       //! TRACE       洋葱红
};

extern "C" {

size_t LogSetMaxLength(size_t max_len)
{
    std::lock_guard<std::mutex> lg(_lock);
    auto origin_len = _LogTextMaxLength;
    _LogTextMaxLength = max_len;
    return origin_len;
}

size_t LogGetMaxLength()
{
    std::lock_guard<std::mutex> lg(_lock);
    return _LogTextMaxLength;
}

/**
 * \brief   日志格式化打印接口的实现
 *
 * 1.对数据合法性进行校验;
 * 2.将日志数据打包成 LogContent，然后调用 _output_channels 指向的函数进行输出
 */
void LogPrintfFunc(const char *module_id, const char *func_name, const char *file_name,
                   int line, int level, int with_args, const char *fmt, ...)
{
    if (CantDispatch())
        return;

    if (level < 0) level = 0;
    if (level >= LOG_LEVEL_MAX) level = (LOG_LEVEL_MAX - 1);

    const char *module_id_be_print = (module_id != nullptr) ? module_id : "???";

    struct timeval tv;
    struct timezone tz;
    gettimeofday(&tv, &tz);

    LogContent content = {
        .thread_id = syscall(SYS_gettid),
        .timestamp = {
            .sec  = static_cast<uint32_t>(tv.tv_sec),
            .usec = static_cast<uint32_t>(tv.tv_usec),
        },
        .module_id = module_id_be_print,
        .func_name = func_name,
        .file_name = Basename(file_name),
        .line = line,
        .level = level,
        .text_len = 0,
        .text_ptr = nullptr,
        .text_trunc = false,
    };

    if (fmt != nullptr) {
        if (with_args) {
            uint32_t buff_size = std::min(2048lu, _LogTextMaxLength) + 1;

            for (;;) {
                va_list args;
                char buffer[buff_size];

                va_start(args, fmt);
                size_t len = 0;

                len = ::vsnprintf(buffer, buff_size, fmt, args);

                if (content.text_trunc)
                    len = _LogTextMaxLength;

                va_end(args);

                //! 如果buffer的空间够用，则正常派发日志
                //! 否则要对buffer空间进行扩张，或是对内容进行截断
                if (len < buff_size) {
                    content.text_len = len;
                    content.text_ptr = buffer;
                    Dispatch(content);
                    break;
                }

                //! 没有超过MaxLength，则进行扩张
                if (len <= _LogTextMaxLength) {
                    buff_size = len + 1;    //! 要多留一个结束符 \0，否则 vsnprintf() 会少一个字符

                } else {    //! 否则进行截断处理
                    buff_size = _LogTextMaxLength + 1;  //! 同上
                    content.text_trunc = true;
                }
            }

        } else {
            content.text_len = ::strlen(fmt);

            //! 如果超出最大长度，要限制
            if (content.text_len > _LogTextMaxLength) {
                content.text_len = _LogTextMaxLength;
                content.text_trunc = true;
            }

            content.text_ptr = fmt;
            Dispatch(content);
        }

    } else {
        Dispatch(content);
    }
}

}

uint32_t LogAddPrintfFunc(LogPrintfFuncType func, void *ptr)
{
    std::lock_guard<std::mutex> lg(_lock);
    uint32_t new_id = ++_id_alloc;
    OutputChannel channel = {
        .id     = new_id,
        .func   = func,
        .ptr    = ptr
    };
    _output_channels.push_back(channel);
    return new_id;
}

bool LogRemovePrintfFunc(uint32_t id)
{
    std::lock_guard<std::mutex> lg(_lock);
    auto iter = std::remove_if(_output_channels.begin(), _output_channels.end(),
        [id](const OutputChannel &item) {
            return (item.id == id);
        }
    );

    if (iter != _output_channels.end()) {
        _output_channels.erase(iter, _output_channels.end());
        return true;
    }
    return false;
}

