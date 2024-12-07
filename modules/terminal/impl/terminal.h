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
#ifndef TBOX_TELNETD_TERMINAL_IMPL_H_20220128
#define TBOX_TELNETD_TERMINAL_IMPL_H_20220128

#include <tbox/base/cabinet.hpp>
#include <tbox/base/object_pool.hpp>
#include <tbox/event/forward.h>

#include "../terminal.h"
#include "node.h"
#include "session_context.h"

namespace tbox {
namespace terminal {

class Terminal::Impl {
  public:
    Impl(event::Loop *wp_loop);
    ~Impl();

  public:
    SessionToken newSession(Connection *wp_conn);
    bool deleteSession(const SessionToken &st);

    uint32_t getOptions(const SessionToken &st) const;
    void setOptions(const SessionToken &st, uint32_t options);

    bool onBegin(const SessionToken &st);
    bool onExit(const SessionToken &st);

    bool onRecvString(const SessionToken &st, const std::string &str);
    bool onRecvWindowSize(const SessionToken &st, uint16_t w, uint16_t h);

  public:
    NodeToken createFuncNode(const Func &func, const std::string &help);
    NodeToken createDirNode(const std::string &help);
    bool deleteNode(NodeToken node_token);
    NodeToken rootNode() const { return root_token_; }
    NodeToken findNode(const std::string &path) const;
    bool mountNode(const NodeToken &parent, const NodeToken &child, const std::string &name);
    bool umountNode(const NodeToken &parent, const std::string &name);

  public:
    void setWelcomeText(const std::string &text) { welcome_text_ = text; }

  protected:
    void onChar(SessionContext *s, char ch);
    void onEnterKey(SessionContext *s);
    void onBackspaceKey(SessionContext *s);
    void onDeleteKey(SessionContext *s);
    void onTabKey(SessionContext *s);
    void onMoveUpKey(SessionContext *s);
    void onMoveDownKey(SessionContext *s);
    void onMoveLeftKey(SessionContext *s);
    void onMoveRightKey(SessionContext *s);
    void onHomeKey(SessionContext *s);
    void onEndKey(SessionContext *s);

    void printPrompt(SessionContext *s);
    void printHelp(SessionContext *s);

    bool execute(SessionContext *s);
    bool executeCmd(SessionContext *s, const std::string &cmdline);

    void executeCdCmd(SessionContext *s, const Args &args);
    void executeHelpCmd(SessionContext *s, const Args &args);
    void executeLsCmd(SessionContext *s, const Args &args);
    void executeHistoryCmd(SessionContext *s, const Args &args);
    void executeExitCmd(SessionContext *s, const Args &args);
    void executeTreeCmd(SessionContext *s, const Args &args);
    void executePwdCmd(SessionContext *s, const Args &args);
    bool executeRunHistoryCmd(SessionContext *s, const Args &args);
    void executeUserCmd(SessionContext *s, const Args &args);

    bool findNode(const std::string &path, Path &node_path) const;

  private:
    event::Loop *wp_loop_ = nullptr;

    ObjectPool<SessionContext> session_ctx_pool_{1};
    cabinet::Cabinet<SessionContext> sessions_;
    cabinet::Cabinet<Node> nodes_;
    NodeToken root_token_;

    std::string welcome_text_;
};

}
}

#endif //TBOX_TELNETD_TERMINAL_IMPL_H_20220128
