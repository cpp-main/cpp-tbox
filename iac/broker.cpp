#include "broker.h"
#include <cassert>

#include <tbox/base/log.h>

namespace tbox::iac {

class Broker::Impl {
  public:
    Impl(event::Loop *wp_loop);

  private:
    event::Loop *wp_loop_ = nullptr;
};

Broker::Broker(event::Loop *wp_loop) :
    impl_(new Impl(wp_loop))
{
    assert(impl_ != nullptr);
}

Broker::~Broker()
{
    delete impl_;
}

Broker::Token Broker::subscribe(const std::string &topic, const Callback &cb)
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

void Broker::cleanup()
{
    LogUndo();
}

}
