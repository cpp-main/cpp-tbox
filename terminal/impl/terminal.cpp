#include "terminal.h"

#include <vector>
#include <deque>

#include <tbox/base/log.h>

namespace tbox::terminal {

class SessionImpl : public Session {
  public:
    SessionImpl(Connection *wp_conn) :
        wp_conn_(wp_conn)
    { }

    ~SessionImpl() { }

    void setSessionToken(const SessionToken &token)
    { token_ = token; }

    void setWindowSize(uint16_t w, uint16_t h)
    { window_width_ = w; window_height_ = h; }

  public:
    bool send(const std::string &str) override { return wp_conn_->send(token_, str); }
    void endSession() override { wp_conn_->endSession(token_); }
    bool isValid() const override { return wp_conn_->isValid(token_); }

    bool window_width() const override { return window_width_; }
    bool window_height() const override { return window_height_; }

  public:
    std::string curr_input;
    size_t cursor = 0;

    std::vector<NodeToken> pwd;        //! 当前路径
    std::deque<std::string> history;   //! 历史命令

  private:
    Connection *wp_conn_ = nullptr;
    SessionToken token_;

    uint16_t window_width_ = 0;
    uint16_t window_height_ = 0;
};

Terminal::Impl::~Impl()
{
    sessions_.foreach(
        [](SessionImpl *p) {
            delete p;
        }
    );
    sessions_.clear();
}

SessionToken Terminal::Impl::newSession(Connection *wp_conn)
{
    auto s = new SessionImpl(wp_conn);
    auto t = sessions_.insert(s);
    s->setSessionToken(t);
    return t;
}

bool Terminal::Impl::deleteSession(const SessionToken &token)
{
    auto s = sessions_.remove(token);
    if (s != nullptr) {
        delete s;
        return true;
    }
    return false;
}

bool Terminal::Impl::onRecvString(const SessionToken &token, const std::string &str)
{
    auto s = sessions_.at(token);
    if (s != nullptr) {
        LogTrace("%s", str.c_str());
        s->send(str);
        return true;
    }
    return false;
}

bool Terminal::Impl::onRecvWindowSize(const SessionToken &token, uint16_t w, uint16_t h)
{
    auto s = sessions_.at(token);
    if (s != nullptr) {
        s->setWindowSize(w, h);
        return true;
    }
    return false;
}

NodeToken Terminal::Impl::create(const EndNode &info)
{
    LogUndo();
    return NodeToken();
}

NodeToken Terminal::Impl::create(const DirNode &info)
{
    LogUndo();
    return NodeToken();
}

NodeToken Terminal::Impl::root() const
{
    LogUndo();
    return NodeToken();
}

NodeToken Terminal::Impl::find(const std::string &path) const
{
    LogUndo();
    return NodeToken();
}

bool Terminal::Impl::mount(const NodeToken &parent, const NodeToken &child, const std::string &name)
{
    LogUndo();
    return false;
}

void Terminal::Impl::onEnterKey(SessionImpl *s)
{
    LogUndo();
}

void Terminal::Impl::onBackspaceKey(SessionImpl *s)
{
    LogUndo();
}

void Terminal::Impl::onTabKey(SessionImpl *s)
{
    LogUndo();
}

void Terminal::Impl::onUpKey(SessionImpl *s)
{
    LogUndo();
}

void Terminal::Impl::onDownKey(SessionImpl *s)
{
    LogUndo();
}

void Terminal::Impl::onLeftKey(SessionImpl *s)
{
    LogUndo();
}

void Terminal::Impl::onRightKey(SessionImpl *s)
{
    LogUndo();
}


}

