#include "terminal.h"
#include <cassert>

#include "impl/terminal.h"

namespace tbox::terminal {

Terminal::Terminal() :
    impl_(new Impl)
{
    assert(impl_ != nullptr);
}

Terminal::~Terminal()
{
    delete impl_;
}

SessionToken Terminal::newSession(Connection *wp_conn)
{
    return impl_->newSession(wp_conn);
}

bool Terminal::deleteSession(const SessionToken &session)
{
    return impl_->deleteSession(session);
}

bool Terminal::input(const SessionToken &session, const std::string &str)
{
    return impl_->input(session, str);
}

NodeToken Terminal::create(const EndNode &info)
{
    return impl_->create(info);
}

NodeToken Terminal::create(const DirNode &info)
{
    return impl_->create(info);
}

NodeToken Terminal::root() const
{
    return impl_->root();
}

NodeToken Terminal::find(const std::string &path) const
{
    return impl_->find(path);
}

bool Terminal::mount(const NodeToken &parent, const NodeToken &child, const std::string &name)
{
    return impl_->mount(parent, child, name);
}

}
