#include "terminal.h"

#include <vector>
#include <deque>

#include <tbox/base/log.h>

#include "session_imp.h"

namespace tbox::terminal {

Terminal::Impl::~Impl()
{
    sessions_.foreach(
        [](SessionImpl *p) {
            delete p;
        }
    );
    sessions_.clear();
}

SessionToken Terminal::Impl::newSession(Connection *wp_conn)
{
    auto s = new SessionImpl(wp_conn);
    auto t = sessions_.insert(s);
    s->setSessionToken(t);
    return t;
}

bool Terminal::Impl::deleteSession(const SessionToken &st)
{
    auto s = sessions_.remove(st);
    if (s != nullptr) {
        delete s;
        return true;
    }
    return false;
}

bool Terminal::Impl::onBegin(const SessionToken &st)
{
    auto s = sessions_.at(st);
    if (s == nullptr)
        return false;

    s->send("\r\nWelcome to TBox Terminal.\r\n$ ");
    return true;
}

bool Terminal::Impl::onExit(const SessionToken &st)
{
    auto s = sessions_.at(st);
    if (s == nullptr)
        return false;

    s->send("Bye!");
    return true;
}

bool Terminal::Impl::onRecvString(const SessionToken &st, const std::string &str)
{
    auto s = sessions_.at(st);
    if (s == nullptr)
        return false;

    LogTrace("%s", str.c_str());
    for (char c : str) {
        if (c == 0x7f) {    //! 退格键
            onBackspaceKey(s);
        } else if (c == 0x09) { //! 制表符
            onTabKey(s);
        } else {
            onChar(s, c);
        }
    }
    return true;
}

bool Terminal::Impl::onRecvWindowSize(const SessionToken &st, uint16_t w, uint16_t h)
{
    auto s = sessions_.at(st);
    if (s != nullptr) {
        s->setWindowSize(w, h);
        return true;
    }
    return false;
}

NodeToken Terminal::Impl::create(const EndNode &info)
{
    LogUndo();
    return NodeToken();
}

NodeToken Terminal::Impl::create(const DirNode &info)
{
    LogUndo();
    return NodeToken();
}

NodeToken Terminal::Impl::root() const
{
    LogUndo();
    return NodeToken();
}

NodeToken Terminal::Impl::find(const std::string &path) const
{
    LogUndo();
    return NodeToken();
}

bool Terminal::Impl::mount(const NodeToken &parent, const NodeToken &child, const std::string &name)
{
    LogUndo();
    return false;
}

void Terminal::Impl::onChar(SessionImpl *s, char ch)
{
    s->send(ch);

    s->cursor++;
    s->curr_input.push_back(ch);
}

void Terminal::Impl::onEnterKey(SessionImpl *s)
{
    LogUndo();
}

void Terminal::Impl::onBackspaceKey(SessionImpl *s)
{
    if (s->cursor == 0)
        return;

    s->send(8);
    s->send(' ');
    s->send(8);

    s->cursor--;
    s->curr_input.pop_back();
}

void Terminal::Impl::onTabKey(SessionImpl *s)
{
    LogUndo();
}

void Terminal::Impl::onUpKey(SessionImpl *s)
{
    LogUndo();
}

void Terminal::Impl::onDownKey(SessionImpl *s)
{
    LogUndo();
}

void Terminal::Impl::onLeftKey(SessionImpl *s)
{
    LogUndo();
}

void Terminal::Impl::onRightKey(SessionImpl *s)
{
    LogUndo();
}


}

