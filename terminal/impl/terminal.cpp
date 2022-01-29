#include "terminal.h"
#include <tbox/base/log.h>

namespace tbox::terminal {

Session Terminal::Impl::newSession(Connection *wp_conn)
{
    LogUndo();
    return Session();
}

bool Terminal::Impl::deleteSession(const Session &session)
{
    LogUndo();
    return false;
}

bool Terminal::Impl::input(const Session &session, const std::string &str)
{
    LogUndo();
    return false;
}

Node Terminal::Impl::create(const EndNode &info)
{
    LogUndo();
    return Node();
}

Node Terminal::Impl::create(const DirNode &info)
{
    LogUndo();
    return Node();
}

Node Terminal::Impl::root() const
{
    LogUndo();
    return Node();
}

Node Terminal::Impl::find(const std::string &path) const
{
    LogUndo();
    return Node();
}

bool Terminal::Impl::mount(const Node &parent, const Node &child, const std::string &name)
{
    LogUndo();
    return false;
}

}

