#include "shell.h"
#include <tbox/base/log.h>

namespace tbox::shell {

Session Shell::Impl::newSession(Connection *wp_conn)
{
    LogUndo();
    return Session();
}

bool Shell::Impl::deleteSession(const Session &session)
{
    LogUndo();
    return false;
}

bool Shell::Impl::input(const Session &session, const std::string &str)
{
    LogUndo();
    return false;
}

Node Shell::Impl::create(const EndNode &info)
{
    LogUndo();
    return Node();
}

Node Shell::Impl::create(const DirNode &info)
{
    LogUndo();
    return Node();
}

Node Shell::Impl::root() const
{
    LogUndo();
    return Node();
}

Node Shell::Impl::find(const std::string &path) const
{
    LogUndo();
    return Node();
}

bool Shell::Impl::mount(const Node &parent, const Node &child, const std::string &name)
{
    LogUndo();
    return false;
}

}

