#include "service.h"
#include <cassert>

#include <tbox/base/log.h>

namespace tbox::iac {

class Service::Impl {
  public:
    Impl(event::Loop *wp_loop);

  private:
    event::Loop *wp_loop_ = nullptr;
};

Service::Service(event::Loop *wp_loop) :
    impl_(new Impl(wp_loop))
{
    assert(impl_ != nullptr);
}

Service::~Service()
{
    delete impl_;
}

Service::Token Service::reg(const std::string &topic, const Callback &cb)
{
    LogUndo();
    return Token();
}

bool Service::unreg(const Token &token)
{
    LogUndo();
    return false;
}

bool Service::invoke(const std::string &topic, const void *in_ptr, void *out_ptr) const
{
    LogUndo();
    return false;
}

bool Service::isExist(const std::string &topic) const
{
    LogUndo();
    return false;
}

void Service::cleanup()
{
    LogUndo();
}

}
