#include "terminal.h"
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

  public:
    bool send(const std::string &str) override {
        return wp_conn_->send(token_, str);
    }

    void endSession() override {
        wp_conn_->endSession(token_);
    }

    bool isValid() const override {
        return wp_conn_->isValid(token_);
    }

  private:
    Connection *wp_conn_ = nullptr;
    SessionToken token_;
};

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

bool Terminal::Impl::input(const SessionToken &token, const std::string &str)
{
    auto s = sessions_.at(token);
    if (s != nullptr) {
        LogUndo();
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

}

