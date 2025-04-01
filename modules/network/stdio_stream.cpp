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
#include "stdio_stream.h"

#include <cstdio>

namespace tbox {
namespace network {

StdinStream::StdinStream(event::Loop *wp_loop) :
    buff_fd_(wp_loop)
{ }

bool StdinStream::initialize()
{
    buff_fd_.initialize(STDIN_FILENO, BufferedFd::kReadOnly);
    return true;
}

bool StdinStream::enable()
{
    return buff_fd_.enable();
}

void StdinStream::setReceiveCallback(const ReceiveCallback &cb, size_t threshold)
{
    buff_fd_.setReceiveCallback(cb, threshold);
}

bool StdinStream::disable()
{
    return buff_fd_.disable();
}

StdoutStream::StdoutStream(event::Loop *wp_loop) :
    buff_fd_(wp_loop)
{ }

bool StdoutStream::initialize()
{
    buff_fd_.initialize(STDOUT_FILENO, BufferedFd::kWriteOnly);
    return true;
}

void StdoutStream::setSendCompleteCallback(const SendCompleteCallback &cb)
{
    buff_fd_.setSendCompleteCallback(cb);
}

bool StdoutStream::send(const void *data_ptr, size_t data_size)
{
    return buff_fd_.send(data_ptr, data_size);
}

bool StdoutStream::enable()
{
    return buff_fd_.enable();
}

bool StdoutStream::disable()
{
    return buff_fd_.disable();
}

StdioStream::StdioStream(event::Loop *wp_loop) :
    in_buff_fd_(wp_loop),
    out_buff_fd_(wp_loop)
{ }

bool StdioStream::initialize()
{
    in_buff_fd_.initialize(STDIN_FILENO, BufferedFd::kReadOnly);
    out_buff_fd_.initialize(STDOUT_FILENO, BufferedFd::kWriteOnly);
    return true;
}

void StdioStream::setReceiveCallback(const ReceiveCallback &cb, size_t threshold)
{
    in_buff_fd_.setReceiveCallback(cb, threshold);
}

void StdioStream::setSendCompleteCallback(const SendCompleteCallback &cb)
{
    out_buff_fd_.setSendCompleteCallback(cb);
}

bool StdioStream::send(const void *data_ptr, size_t data_size)
{
    return out_buff_fd_.send(data_ptr, data_size);
}

bool StdioStream::enable()
{
    return in_buff_fd_.enable() && out_buff_fd_.enable();
}

bool StdioStream::disable()
{
    return in_buff_fd_.disable() && out_buff_fd_.disable();
}

}
}
