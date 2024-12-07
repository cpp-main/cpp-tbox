/*
 *     .============.
 *    //  M A K E  / \
 *   //  C++ DEV  /   \
 *  //  E A S Y  /  \/ \
 * ++ ----------.  \/\  .
 *  \\     \     \ /\  /
 *   \\     \     \   /
 *    \\     \     \ /
 *     -============'
 *
 * Copyright (c) 2018 Hevake and contributors, all rights reserved.
 *
 * This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
 * Use of this source code is governed by MIT license that can be found
 * in the LICENSE file in the root of the source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */
#include "terminal.h"
#include <tbox/base/assert.h>

#include "impl/terminal.h"

namespace tbox {
namespace terminal {

Terminal::Terminal(event::Loop *wp_loop) :
    impl_(new Impl(wp_loop))
{
    TBOX_ASSERT(impl_ != nullptr);
}

Terminal::~Terminal()
{
    delete impl_;
}

void Terminal::setWelcomeText(const std::string &welcome_text)
{
    impl_->setWelcomeText(welcome_text);
}

SessionToken Terminal::newSession(Connection *wp_conn)
{
    return impl_->newSession(wp_conn);
}

bool Terminal::deleteSession(const SessionToken &st)
{
    return impl_->deleteSession(st);
}

uint32_t Terminal::getOptions(const SessionToken &st) const
{
    return impl_->getOptions(st);
}

void Terminal::setOptions(const SessionToken &st, uint32_t options)
{
    impl_->setOptions(st, options);
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

bool Terminal::deleteNode(NodeToken node_token)
{
    return impl_->deleteNode(node_token);
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

bool Terminal::umountNode(const NodeToken &parent, const std::string &name)
{
    return impl_->umountNode(parent, name);
}

}
}
