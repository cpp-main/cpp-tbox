#include "shell.h"
#include <cassert>

#include "impl/shell.h"

namespace tbox::shell {

Shell::Shell() :
    impl_(new Impl)
{
    assert(impl_ != nullptr);
}

Shell::~Shell()
{
    delete impl_;
}

Session Shell::newSession(Connection *wp_conn)
{
    return impl_->newSession(wp_conn);
}

bool Shell::deleteSession(const Session &session)
{
    return impl_->deleteSession(session);
}

bool Shell::input(const Session &session, const std::string &str)
{
    return impl_->input(session, str);
}

Node Shell::create(const EndNode &info)
{
    return impl_->create(info);
}

Node Shell::create(const DirNode &info)
{
    return impl_->create(info);
}

Node Shell::root() const
{
    return impl_->root();
}

Node Shell::find(const std::string &path) const
{
    return impl_->find(path);
}

bool Shell::mount(const Node &parent, const Node &child, const std::string &name)
{
    return impl_->mount(parent, child, name);
}

}
