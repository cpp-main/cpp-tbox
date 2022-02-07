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

bool Terminal::deleteSession(const SessionToken &st)
{
    return impl_->deleteSession(st);
}

bool Terminal::onBegin(const SessionToken &st)
{
    return impl_->onBegin(st);
}

bool Terminal::onExit(const SessionToken &st)
{
    return impl_->onExit(st);
}

bool Terminal::onRecvString(const SessionToken &st, const std::string &str)
{
    return impl_->onRecvString(st, str);
}

bool Terminal::onRecvWindowSize(const SessionToken &st, uint16_t w, uint16_t h)
{
    return impl_->onRecvWindowSize(st, w, h);
}

NodeToken Terminal::create(const FuncInfo &info)
{
    return impl_->create(info);
}

NodeToken Terminal::create(const DirInfo &info)
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

bool Terminal::mount(const NodeToken &parent, const NodeToken &child)
{
    return impl_->mount(parent, child);
}

bool Terminal::list(const NodeToken &token, std::vector<NodeInfo> &node_vec) const
{
    return impl_->list(token, node_vec);
}

}
