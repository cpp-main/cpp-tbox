#ifndef TBOX_TELNETD_TERMINAL_IMPL_H_20220128
#define TBOX_TELNETD_TERMINAL_IMPL_H_20220128

#include <tbox/base/cabinet.hpp>

#include "../terminal.h"
#include "node.h"

namespace tbox::terminal {

class SessionContext;

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

    bool executeCmdline(SessionContext *s);

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
    cabinet::Cabinet<SessionContext> sessions_;
    cabinet::Cabinet<Node> nodes_;
    NodeToken root_token_;
};

}

#endif //TBOX_TELNETD_TERMINAL_IMPL_H_20220128
