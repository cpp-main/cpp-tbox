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

NodeToken Terminal::createFuncNode(const Func &func, const std::string &help)
{
    return impl_->createFuncNode(func, help);
}

NodeToken Terminal::createDirNode(const std::string &help)
{
    return impl_->createDirNode(help);
}

NodeToken Terminal::rootNode() const
{
    return impl_->rootNode();
}

NodeToken Terminal::findNode(const std::string &path) const
{
    return impl_->findNode(path);
}

bool Terminal::mountNode(const NodeToken &parent, const NodeToken &child, const std::string &name)
{
    return impl_->mountNode(parent, child, name);
}

}
