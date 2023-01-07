#include "broker.h"

#include <tbox/base/assert.h>
#include <tbox/base/log.h>

namespace tbox {
namespace broker {

class Broker::Impl {
  public:
    Impl(event::Loop *wp_loop);
    ~Impl();

  public:
    Token subscribe(const std::string &topic, const MessageCallback &cb);
    bool unsubscribe(const Token &token);
    void publish(const std::string &topic, const void *msg_ptr, size_t msg_size);

    Token reg(const std::string &topic, const RequestCallback &cb);
    bool unreg(const Token &token);
    size_t invoke(const std::string &topic, const void *in_ptr, void *out_ptr) const;

    void cleanup();

  private:
    event::Loop *wp_loop_ = nullptr;
};

Broker::Broker(event::Loop *wp_loop) :
    impl_(new Impl(wp_loop))
{
    TBOX_ASSERT(impl_ != nullptr);
}

Broker::~Broker()
{
    delete impl_;
}

Broker::Token Broker::subscribe(const std::string &topic, const MessageCallback &cb)
{
    return impl_->subscribe(topic, cb);
}

bool Broker::unsubscribe(const Token &token)
{
    return impl_->unsubscribe(token);
}

void Broker::publish(const std::string &topic, const void *msg_ptr, size_t msg_size)
{
    impl_->publish(topic, msg_ptr, msg_size);
}

Broker::Token Broker::reg(const std::string &topic, const RequestCallback &cb)
{
    return impl_->reg(topic, cb);
}

bool Broker::unreg(const Token &token)
{
    return impl_->unreg(token);
}

size_t Broker::invoke(const std::string &topic, const void *in_ptr, void *out_ptr) const
{
    return impl_->invoke(topic, in_ptr, out_ptr);
}

void Broker::cleanup()
{
    impl_->cleanup();
}

Broker::Impl::Impl(event::Loop *wp_loop) :
    wp_loop_(wp_loop)
{
    LogUndo();
}

Broker::Impl::~Impl()
{
    LogUndo();
}

Broker::Token Broker::Impl::subscribe(const std::string &topic, const MessageCallback &cb)
{
    LogUndo();
    return Broker::Token();
}

bool Broker::Impl::unsubscribe(const Token &token)
{
    LogUndo();
    return false;
}

void Broker::Impl::publish(const std::string &topic, const void *msg_ptr, size_t msg_size)
{
    LogUndo();
}

Broker::Token Broker::Impl::reg(const std::string &topic, const RequestCallback &cb)
{
    LogUndo();
    return Token();
}

bool Broker::Impl::unreg(const Token &token)
{
    LogUndo();
    return false;
}

size_t Broker::Impl::invoke(const std::string &topic, const void *in_ptr, void *out_ptr) const
{
    LogUndo();
    return 0;
}

void Broker::Impl::cleanup()
{
    LogUndo();
}

}
}
