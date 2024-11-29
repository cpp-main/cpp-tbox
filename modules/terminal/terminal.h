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
#ifndef TBOX_TELNETD_TERMINAL_H_20220128
#define TBOX_TELNETD_TERMINAL_H_20220128

#include "terminal_interact.h"
#include "terminal_nodes.h"

#include <tbox/event/forward.h>

namespace tbox {
namespace terminal {

class Terminal : public TerminalInteract,
                 public TerminalNodes {
  public:
    Terminal(event::Loop *wp_loop);
    virtual ~Terminal() override;

  public:
    virtual SessionToken newSession(Connection *wp_conn) override;
    virtual bool deleteSession(const SessionToken &st) override;

    virtual uint32_t getOptions(const SessionToken &st) const override;
    virtual void setOptions(const SessionToken &st, uint32_t options) override;

    virtual bool onBegin(const SessionToken &st) override;
    virtual bool onRecvString(const SessionToken &st, const std::string &str) override;
    virtual bool onRecvWindowSize(const SessionToken &st, uint16_t w, uint16_t h) override;
    virtual bool onExit(const SessionToken &st) override;

  public:
    virtual NodeToken createFuncNode(const Func &func, const std::string &help) override;
    virtual NodeToken createDirNode(const std::string &help) override;
    virtual bool deleteNode(NodeToken node_token) override;
    virtual NodeToken rootNode() const override;
    virtual NodeToken findNode(const std::string &path) const override;
    virtual bool mountNode(const NodeToken &parent, const NodeToken &child, const std::string &name) override;
    virtual bool umountNode(const NodeToken &parent, const std::string &name) override;

  public:
    void setWelcomeText(const std::string &text);

  private:
    class Impl;
    Impl *impl_ = nullptr;
};

}
}

#endif //TBOX_TELNETD_TERMINAL_H_20220128
