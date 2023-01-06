#include "broker.h"

#include <tbox/base/assert.h>
#include <tbox/base/log.h>

namespace tbox {
namespace broker {

class Broker::Impl {
  public:
    Impl(event::Loop *wp_loop);

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
    LogUndo();
    return Token();
}

bool Broker::unsubscribe(const Token &token)
{
    LogUndo();
    return false;
}

void Broker::publish(const std::string &topic, const void *msg_ptr, size_t msg_size)
{
    LogUndo();
}

Broker::Token Broker::reg(const std::string &topic, const RequestCallback &cb)
{
    LogUndo();
    return Token();
}

bool Broker::unreg(const Token &token)
{
    LogUndo();
    return false;
}

size_t Broker::invoke(const std::string &topic, const void *in_ptr, void *out_ptr) const
{
    LogUndo();
    return 0;
}

void Broker::cleanup()
{
    LogUndo();
}

}
}
