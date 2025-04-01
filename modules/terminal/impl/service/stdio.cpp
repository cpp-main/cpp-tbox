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
 * Copyright (c) 2025 Hevake and contributors, all rights reserved.
 *
 * This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
 * Use of this source code is governed by MIT license that can be found
 * in the LICENSE file in the root of the source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */

#include "stdio.h"

namespace tbox {
namespace terminal {

using namespace std::placeholders;

Stdio::Impl::Impl(event::Loop *wp_loop, TerminalInteract *wp_terminal) :
    wp_loop_(wp_loop),
    wp_terminal_(wp_terminal),
    stdio_stream_(wp_loop)
{ }

bool Stdio::Impl::initialize()
{
    //!NOTE: 对终端的设置一定要放到stdio_stream_.initialize()前面，否则会导致stdio_stream_失效
    disableTermiosBuffer();

    stdio_stream_.initialize();
    stdio_stream_.setReceiveCallback(std::bind(&Impl::onStdinRecv, this, _1), 1);

    return true;
}

bool Stdio::Impl::start()
{
    if (stdio_stream_.enable()) {
        startSession();
        return true;
    }

    return false;
}

void Stdio::Impl::stop()
{
    stdio_stream_.disable();
    wp_terminal_->deleteSession(session_token_);
    session_token_.reset();
}

void Stdio::Impl::cleanup()
{
    restoreTermiosBuffer();
}

void Stdio::Impl::startSession()
{
    session_token_ = wp_terminal_->newSession(this);
    wp_terminal_->setOptions(session_token_, TerminalInteract::kEnableEcho);
    wp_terminal_->onBegin(session_token_);
}

bool Stdio::Impl::send(const SessionToken &, const std::string &str)
{
    return stdio_stream_.send(str.c_str(), str.size());
}

bool Stdio::Impl::send(const SessionToken &, char ch)
{
    return stdio_stream_.send(&ch, 1);
}

bool Stdio::Impl::endSession(const SessionToken &st)
{
    session_token_.reset();
    return true;
}

bool Stdio::Impl::isValid(const SessionToken &st) const
{
    return session_token_ == st;
}

void Stdio::Impl::onStdinRecv(util::Buffer& buff)
{
    if (!session_token_.isNull()) {
        std::string str(reinterpret_cast<const char *>(buff.readableBegin()), buff.readableSize());
        wp_terminal_->onRecvString(session_token_, str);

    } else {
        startSession();
    }

    buff.hasReadAll();
}

//! 关闭终端的缓存与回显
void Stdio::Impl::disableTermiosBuffer()
{
    struct termios new_settings;
    ::tcgetattr(STDIN_FILENO, &origin_settings_);

    new_settings = origin_settings_;
    new_settings.c_lflag &= ~(ICANON | ECHO);

    ::tcsetattr(STDIN_FILENO, TCSANOW, &new_settings);
}

//! 恢复终端的缓存与回显
void Stdio::Impl::restoreTermiosBuffer()
{
    ::tcsetattr(STDIN_FILENO, TCSANOW, &origin_settings_);
}

}
}
