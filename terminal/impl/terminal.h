#ifndef TBOX_TELNETD_TERMINAL_IMPL_H_20220128
#define TBOX_TELNETD_TERMINAL_IMPL_H_20220128

#include "../terminal.h"
#include <tbox/base/cabinet.hpp>

namespace tbox::terminal {

class SessionImpl;

class Terminal::Impl {
  public:
    ~Impl();

  public:
    SessionToken newSession(Connection *wp_conn);
    bool deleteSession(const SessionToken &st);

    bool onBegin(const SessionToken &st);
    bool onExit(const SessionToken &st);

    bool onRecvString(const SessionToken &st, const std::string &str);
    bool onRecvWindowSize(const SessionToken &st, uint16_t w, uint16_t h);

  public:
    NodeToken create(const EndNode &info);
    NodeToken create(const DirNode &info);
    NodeToken root() const;
    NodeToken find(const std::string &path) const;
    bool mount(const NodeToken &parent, const NodeToken &child, const std::string &name);

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
    bool executeCmd(SessionImpl *s);

  private:
    cabinet::Cabinet<SessionImpl> sessions_;
};

}

#endif //TBOX_TELNETD_TERMINAL_IMPL_H_20220128
