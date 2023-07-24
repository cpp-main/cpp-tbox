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
#include "async_stdout_sink.h"

#include <unistd.h>
#include <algorithm>

namespace tbox {
namespace log {

AsyncStdoutSink::AsyncStdoutSink()
{
    AsyncSink::Config cfg;
    cfg.buff_size = 10240;
    cfg.buff_min_num = 2;
    cfg.buff_max_num = 20;
    cfg.interval  = 100;

    setConfig(cfg);
}

void AsyncStdoutSink::appendLog(const char *str, size_t len)
{
    buffer_.reserve(buffer_.size() + len - 1);
    std::back_insert_iterator<std::vector<char>>  back_insert_iter(buffer_);
    std::copy(str, str + len - 1, back_insert_iter);
}

void AsyncStdoutSink::flushLog()
{
    auto wsize = ::write(STDOUT_FILENO, buffer_.data(), buffer_.size()); //! 写到终端
    (void)wsize;  //! 消除警告用

    buffer_.clear();
    if (buffer_.capacity() > 1024)
        buffer_.shrink_to_fit();
}

}
}
