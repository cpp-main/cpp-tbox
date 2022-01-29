#include "terminal.h"
#include <tbox/base/log.h>

namespace tbox::terminal {

SessionToken Terminal::Impl::newSession(Connection *wp_conn)
{
    LogUndo();
    return SessionToken();
}

bool Terminal::Impl::deleteSession(const SessionToken &session)
{
    LogUndo();
    return false;
}

bool Terminal::Impl::input(const SessionToken &session, const std::string &str)
{
    LogUndo();
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

}

