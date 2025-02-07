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
#include "terminal.h"

#include <sstream>

#include <tbox/base/log.h>

#include "../connection.h"
#include "dir_node.h"
#include "func_node.h"

namespace tbox {
namespace terminal {

using namespace std;

Terminal::Impl::Impl(event::Loop *wp_loop)
    : wp_loop_(wp_loop)
{
    welcome_text_ = \
        "\r\n"
        R"(      .============.      )""\r\n"
        R"(     //  M A K E  / \     )""\r\n"
        R"(    //  C++ DEV  /   \    )""\r\n"
        R"(   //  E A S Y  /  \/ \   )""\r\n"
        R"(  ++ ----------.  \/\  .  )""\r\n"
        R"(   \\     \     \ /\  /   )""\r\n"
        R"(    \\     \     \   /    )""\r\n"
        R"(     \\     \     \ /     )""\r\n"
        R"(      -============'      )""\r\n"
        "\r\n"
        "Welcome to cpp-tbox terminal.\r\n";

    root_token_ = nodes_.alloc(new DirNode("this is root node"));
}

Terminal::Impl::~Impl()
{
    sessions_.foreach(
        [this](SessionContext *s) {
            session_ctx_pool_.free(s);
        }
    );
    sessions_.clear();

    nodes_.foreach(
        [](Node *p) {
            delete p;
        }
    );
    nodes_.clear();
}

SessionToken Terminal::Impl::newSession(Connection *wp_conn)
{
    auto s = session_ctx_pool_.alloc();
    auto t = sessions_.alloc(s);
    s->wp_conn = wp_conn;
    s->token = t;
    return t;
}

bool Terminal::Impl::deleteSession(const SessionToken &st)
{
    auto s = sessions_.free(st);
    if (s != nullptr) {
        session_ctx_pool_.free(s);
        return true;
    }
    return false;
}

uint32_t Terminal::Impl::getOptions(const SessionToken &st) const
{
    auto s = sessions_.at(st);
    if (s == nullptr)
        return 0;

    return s->options;
}

void Terminal::Impl::setOptions(const SessionToken &st, uint32_t options)
{
    auto s = sessions_.at(st);
    if (s == nullptr)
        return;

    s->options = options;
}

bool Terminal::Impl::onBegin(const SessionToken &st)
{
    auto s = sessions_.at(st);
    if (s == nullptr)
        return false;

    if (!(s->options & kQuietMode)) {
        s->wp_conn->send(st, welcome_text_);
        s->wp_conn->send(st, "Type 'help' for more information.\r\n\r\n");
    }

    printPrompt(s);

    return true;
}

bool Terminal::Impl::onExit(const SessionToken &st)
{
    auto s = sessions_.at(st);
    if (s == nullptr)
        return false;

    s->wp_conn->send(st, "Bye!");
    return true;
}

bool Terminal::Impl::onRecvString(const SessionToken &st, const string &str)
{
    auto s = sessions_.at(st);
    if (s == nullptr)
        return false;

    s->key_event_scanner_.start();
    KeyEventScanner::Status status = KeyEventScanner::Status::kUnsure;
    for (char c : str) {
        status = s->key_event_scanner_.next(c);
        if (status == KeyEventScanner::Status::kEnsure) {
            switch (s->key_event_scanner_.result()) {
                case KeyEventScanner::Result::kPrintable:
                    onChar(s, c);
                    break;
                case KeyEventScanner::Result::kEnter:
                    onEnterKey(s);
                    break;
                case KeyEventScanner::Result::kBackspace:
                    onBackspaceKey(s);
                    break;
                case KeyEventScanner::Result::kTab:
                    onTabKey(s);
                    break;
                case KeyEventScanner::Result::kMoveUp:
                    onMoveUpKey(s);
                    break;
                case KeyEventScanner::Result::kMoveDown:
                    onMoveDownKey(s);
                    break;
                case KeyEventScanner::Result::kMoveLeft:
                    onMoveLeftKey(s);
                    break;
                case KeyEventScanner::Result::kMoveRight:
                    onMoveRightKey(s);
                    break;
                case KeyEventScanner::Result::kHome:
                    onHomeKey(s);
                    break;
                case KeyEventScanner::Result::kEnd:
                    onEndKey(s);
                    break;
                case KeyEventScanner::Result::kDelete:
                    onDeleteKey(s);
                    break;
                default:
                    break;
            }
            s->key_event_scanner_.start();
        }
    }

    if (status == KeyEventScanner::Status::kUnsure) {
        if (s->key_event_scanner_.stop() == KeyEventScanner::Status::kEnsure) {
            switch (s->key_event_scanner_.result()) {
                case KeyEventScanner::Result::kEnter:
                    onEnterKey(s);
                    break;

                default:
                    break;
            }
        }
    }
    return true;
}

bool Terminal::Impl::onRecvWindowSize(const SessionToken &st, uint16_t w, uint16_t h)
{
    auto s = sessions_.at(st);
    if (s != nullptr) {
        s->window_width = w;
        s->window_height = h;
        return true;
    }
    return false;
}

void Terminal::Impl::printPrompt(SessionContext *s)
{
    if (!(s->options & kQuietMode))
        s->wp_conn->send(s->token, "# ");
}

void Terminal::Impl::printHelp(SessionContext *s)
{
    const char *help_str = \
        "This terminal is designed by Hevake Lee <hevake@126.com>, integrated in cpp-tbox.\r\n"
        "Repository: https://github.com/cpp-main/cpp-tbox\r\n"
        "It provides a way to interact with the program in the form of shell.\r\n"
        "\r\n"
        "There are some buildin commands:\r\n"
        "\r\n"
        "- tree      List all nodes as tree\r\n"
        "- ls        List nodes under specified path\r\n"
        "- cd        Chang directory\r\n"
        "- pwd       print current path\r\n"
        "- exit|quit Exit this\r\n"
        "- help      Print help of specified node\r\n"
        "- history   List history command\r\n"
        "- !n        Run the n command of history\r\n"
        "- !-n       Run the last n command of history\r\n"
        "- !!        Run last command\r\n"
        "\r\n";
    s->wp_conn->send(s->token, help_str);

    if (s->options & kEnableEcho) {
        const char *extra_str = \
            "Besides, UP,DOWN,LEFT,RIGHT,HOME,END,DELETE keys are available.\r\n"
            "Try them.\r\n"
            "\r\n"
            ;
        s->wp_conn->send(s->token, extra_str);
    }
}

}
}
