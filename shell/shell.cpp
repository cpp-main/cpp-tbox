#include "shell.h"
#include <cassert>
#include <tbox/base/log.h>

#include "impl/shell.h"

namespace tbox::telnetd {

Shell::Shell() :
    impl_(new Impl)
{
    assert(impl_ != nullptr);
}

Shell::~Shell()
{
    delete impl_;
}

void Shell::setOutputFunc(const OutputFunc &func)
{
    LogUndo();
}

Session Shell::newSession()
{
    LogUndo();
    return Session();
}

bool Shell::deleteSession(const Session &session)
{
    LogUndo();
    return false;
}

bool Shell::input(const Session &session, const std::string &input)
{
    LogUndo();
    return "";
}

Node Shell::create(const EndNode &info)
{
    LogUndo();
    return Node();
}

Node Shell::create(const DirNode &info)
{
    LogUndo();
    return Node();
}

Node Shell::root() const
{
    LogUndo();
    return Node();
}

Node Shell::find(const std::string &path) const
{
    LogUndo();
    return Node();
}

bool Shell::mount(const Node &parent, const Node &child, const std::string &name)
{
    LogUndo();
    return false;
}

}
