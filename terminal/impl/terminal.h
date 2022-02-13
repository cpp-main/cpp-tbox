#ifndef TBOX_TELNETD_TERMINAL_IMPL_H_20220128
#define TBOX_TELNETD_TERMINAL_IMPL_H_20220128

#include <tbox/base/cabinet.hpp>

#include "../terminal.h"
#include "node.h"

namespace tbox::terminal {

class SessionImpl;

class Terminal::Impl {
  public:
    Impl();
    ~Impl();

  public:
    SessionToken newSession(Connection *wp_conn);
    bool deleteSession(const SessionToken &st);

    bool onBegin(const SessionToken &st);
    bool onExit(const SessionToken &st);

    bool onRecvString(const SessionToken &st, const std::string &str);
    bool onRecvWindowSize(const SessionToken &st, uint16_t w, uint16_t h);

  public:
    NodeToken createFuncNode(const Func &func, const std::string &help);
    NodeToken createDirNode(const std::string &help);
    NodeToken rootNode() const;
    NodeToken findNode(const std::string &path) const;
    bool mountNode(const NodeToken &parent, const NodeToken &child, const std::string &name);

  protected:
    void onChar(SessionImpl *s, char ch);
    void onEnterKey(SessionImpl *s);
    void onBackspaceKey(SessionImpl *s);
    void onDeleteKey(SessionImpl *s);
    void onTabKey(SessionImpl *s);
    void onMoveUpKey(SessionImpl *s);
    void onMoveDownKey(SessionImpl *s);
    void onMoveLeftKey(SessionImpl *s);
    void onMoveRightKey(SessionImpl *s);
    void onHomeKey(SessionImpl *s);
    void onEndKey(SessionImpl *s);

    void printPrompt(SessionImpl *s);
    void printHelp(SessionImpl *s);

    bool executeCmdline(SessionImpl *s);

    void executeCdCmd(SessionImpl *s, const Args &args);
    void executeHelpCmd(SessionImpl *s, const Args &args);
    void executeLsCmd(SessionImpl *s, const Args &args);
    void executeHistoryCmd(SessionImpl *s, const Args &args);
    void executeExitCmd(SessionImpl *s, const Args &args);
    void executeTreeCmd(SessionImpl *s, const Args &args);
    void executePwdCmd(SessionImpl *s, const Args &args);
    bool executeRunHistoryCmd(SessionImpl *s, const Args &args);
    void executeUserCmd(SessionImpl *s, const Args &args);

    bool findNode(const std::string &path, Path &node_path) const;

  private:
    cabinet::Cabinet<SessionImpl> sessions_;
    cabinet::Cabinet<Node> nodes_;
    NodeToken root_token_;
};

}

#endif //TBOX_TELNETD_TERMINAL_IMPL_H_20220128
