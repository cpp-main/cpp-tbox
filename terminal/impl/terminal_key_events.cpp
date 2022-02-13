#include "terminal.h"

#include <sstream>

#include <tbox/base/log.h>

#include "session_imp.h"
#include "dir_node.h"
#include "func_node.h"

namespace tbox::terminal {

using namespace std;

namespace {
    const string MOVE_LEFT_KEY("\033[D");
    const string MOVE_RIGHT_KEY("\033[C");
    const size_t HISTORY_MAX_SIZE(20);
}

void Terminal::Impl::onChar(SessionImpl *s, char ch)
{
    s->send(ch);

    if (s->cursor == s->curr_input.size())
        s->curr_input.push_back(ch);
    else
        s->curr_input.insert(s->cursor, 1, ch);
    s->cursor++;

    stringstream ss;
    ss  << s->curr_input.substr(s->cursor)
        << string((s->curr_input.size() - s->cursor), '\b');
    s->send(ss.str());
}

void Terminal::Impl::onEnterKey(SessionImpl *s)
{
    s->send("\r\n");

    executeCmdline(s);

    printPrompt(s);

    if (!s->curr_input.empty()) {
        s->history.push_back(s->curr_input);
        if (s->history.size() > HISTORY_MAX_SIZE)
            s->history.pop_front();
    }

    s->curr_input.clear();
    s->cursor = 0;
    s->history_index = 0;
}

void Terminal::Impl::onBackspaceKey(SessionImpl *s)
{
    if (s->cursor == 0)
        return;

    if (s->cursor == s->curr_input.size())
        s->curr_input.pop_back();
    else
        s->curr_input.erase((s->cursor - 1), 1);

    s->cursor--;

    stringstream ss;
    ss  << '\b' << s->curr_input.substr(s->cursor) << ' '
        << string((s->curr_input.size() - s->cursor + 1), '\b');
    s->send(ss.str());
}

void Terminal::Impl::onDeleteKey(SessionImpl *s)
{
    if (s->cursor >= s->curr_input.size())
        return;

    s->curr_input.erase((s->cursor), 1);

    stringstream ss;
    ss  << s->curr_input.substr(s->cursor) << ' '
        << string((s->curr_input.size() - s->cursor + 1), '\b');
    s->send(ss.str());
}

void Terminal::Impl::onTabKey(SessionImpl *s)
{
    //!TODO: 实现补全功能
    LogUndo();
}

void Terminal::Impl::onMoveUpKey(SessionImpl *s)
{
    if (s->history_index == s->history.size())
        return;

    while (s->cursor--)
        s->send("\b \b");

    s->history_index++;
    s->curr_input = s->history[s->history.size() - s->history_index];
    s->cursor = s->curr_input.size();
    s->send(s->curr_input);
}

void Terminal::Impl::onMoveDownKey(SessionImpl *s)
{
    if (s->history_index == 0)
        return;

    while (s->cursor--)
        s->send("\b \b");

    s->history_index--;
    if (s->history_index > 0) {
        s->curr_input = s->history[s->history.size() - s->history_index];
        s->cursor = s->curr_input.size();
    } else {
        s->curr_input.clear();
        s->cursor = 0;
    }
    s->send(s->curr_input);
}

void Terminal::Impl::onMoveLeftKey(SessionImpl *s)
{
    if (s->cursor == 0)
        return;

    s->cursor--;
    s->send(MOVE_LEFT_KEY);
}

void Terminal::Impl::onMoveRightKey(SessionImpl *s)
{
    if (s->cursor >= s->curr_input.size())
        return;

    s->cursor++;
    s->send(MOVE_RIGHT_KEY);
}

void Terminal::Impl::onHomeKey(SessionImpl *s)
{
    while (s->cursor != 0) {
        s->send(MOVE_LEFT_KEY);
        s->cursor--;
    }
}

void Terminal::Impl::onEndKey(SessionImpl *s)
{
    while (s->cursor < s->curr_input.size()) {
        s->send(MOVE_RIGHT_KEY);
        s->cursor++;
    }
}

}
