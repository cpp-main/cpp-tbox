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

Session Terminal::newSession(Connection *wp_conn)
{
    return impl_->newSession(wp_conn);
}

bool Terminal::deleteSession(const Session &session)
{
    return impl_->deleteSession(session);
}

bool Terminal::input(const Session &session, const std::string &str)
{
    return impl_->input(session, str);
}

Node Terminal::create(const EndNode &info)
{
    return impl_->create(info);
}

Node Terminal::create(const DirNode &info)
{
    return impl_->create(info);
}

Node Terminal::root() const
{
    return impl_->root();
}

Node Terminal::find(const std::string &path) const
{
    return impl_->find(path);
}

bool Terminal::mount(const Node &parent, const Node &child, const std::string &name)
{
    return impl_->mount(parent, child, name);
}

}
