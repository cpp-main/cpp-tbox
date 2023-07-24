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

#include "session_context.h"
#include "dir_node.h"
#include "func_node.h"
#include "../connection.h"

namespace tbox {
namespace terminal {

using namespace std;

namespace {
    const string MOVE_LEFT_KEY("\033[D");
    const string MOVE_RIGHT_KEY("\033[C");
    const size_t HISTORY_MAX_SIZE(20);
}

void Terminal::Impl::onChar(SessionContext *s, char ch)
{
    if (s->cursor == s->curr_input.size())
        s->curr_input.push_back(ch);
    else
        s->curr_input.insert(s->cursor, 1, ch);
    s->cursor++;

    if (s->options & kEnableEcho) {
        s->wp_conn->send(s->token, ch);

        stringstream ss;
        ss  << s->curr_input.substr(s->cursor)
            << string((s->curr_input.size() - s->cursor), '\b');

        auto refresh_str = ss.str();
        if (!refresh_str.empty())
            s->wp_conn->send(s->token, refresh_str);
    }
}

void Terminal::Impl::onEnterKey(SessionContext *s)
{
    if (s->options & kEnableEcho)
        s->wp_conn->send(s->token, "\r\n");

    if (execute(s)) {
        s->history.push_back(s->curr_input);
        if (s->history.size() > HISTORY_MAX_SIZE)
            s->history.pop_front();
    }

    printPrompt(s);

    s->curr_input.clear();
    s->cursor = 0;
    s->history_index = 0;
}

void Terminal::Impl::onBackspaceKey(SessionContext *s)
{
    if (s->cursor == 0)
        return;

    if (s->cursor == s->curr_input.size())
        s->curr_input.pop_back();
    else
        s->curr_input.erase((s->cursor - 1), 1);

    s->cursor--;

    if (s->options & kEnableEcho) {
        stringstream ss;
        ss  << '\b' << s->curr_input.substr(s->cursor) << ' '
            << string((s->curr_input.size() - s->cursor + 1), '\b');

        s->wp_conn->send(s->token, ss.str());
    }
}

void Terminal::Impl::onDeleteKey(SessionContext *s)
{
    if (s->cursor >= s->curr_input.size())
        return;

    s->curr_input.erase((s->cursor), 1);

    if (s->options & kEnableEcho) {
        stringstream ss;
        ss  << s->curr_input.substr(s->cursor) << ' '
            << string((s->curr_input.size() - s->cursor + 1), '\b');

        s->wp_conn->send(s->token, ss.str());
    }
}

void Terminal::Impl::onTabKey(SessionContext *s)
{
    //!TODO: 实现补全功能
    LogUndo();
    (void)s;
}

namespace {
void CleanupInput(SessionContext *s)
{
    while (s->cursor < s->curr_input.size()) {
        s->wp_conn->send(s->token, MOVE_RIGHT_KEY);
        s->cursor++;
    }

    while (s->cursor--)
        s->wp_conn->send(s->token, "\b \b");
}
}

void Terminal::Impl::onMoveUpKey(SessionContext *s)
{
    if (s->history_index == s->history.size())
        return;

    CleanupInput(s);

    s->history_index++;
    s->curr_input = s->history[s->history.size() - s->history_index];
    s->cursor = s->curr_input.size();

    s->wp_conn->send(s->token, s->curr_input);
}

void Terminal::Impl::onMoveDownKey(SessionContext *s)
{
    if (s->history_index == 0)
        return;

    CleanupInput(s);

    s->history_index--;
    if (s->history_index > 0) {
        s->curr_input = s->history[s->history.size() - s->history_index];
        s->cursor = s->curr_input.size();
    } else {
        s->curr_input.clear();
        s->cursor = 0;
    }

    s->wp_conn->send(s->token, s->curr_input);
}

void Terminal::Impl::onMoveLeftKey(SessionContext *s)
{
    if (s->cursor == 0)
        return;

    s->cursor--;
    s->wp_conn->send(s->token, MOVE_LEFT_KEY);
}

void Terminal::Impl::onMoveRightKey(SessionContext *s)
{
    if (s->cursor >= s->curr_input.size())
        return;

    s->cursor++;
    s->wp_conn->send(s->token, MOVE_RIGHT_KEY);
}

void Terminal::Impl::onHomeKey(SessionContext *s)
{
    while (s->cursor != 0) {
        s->wp_conn->send(s->token, MOVE_LEFT_KEY);
        s->cursor--;
    }
}

void Terminal::Impl::onEndKey(SessionContext *s)
{
    while (s->cursor < s->curr_input.size()) {
        s->wp_conn->send(s->token, MOVE_RIGHT_KEY);
        s->cursor++;
    }
}

}
}
