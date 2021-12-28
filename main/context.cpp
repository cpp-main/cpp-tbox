#include "context.h"

#include <cassert>
#include <tbox/base/json.hpp>

namespace tbox::main {

struct Context::Data {
    event::Loop *sp_loop = nullptr;
    eventx::ThreadPool *sp_thread_pool = nullptr;
};

Context::Context() :
    d_(new Data)
{
    assert(d_ != nullptr);

    d_->sp_loop = event::Loop::New();
    assert(d_->sp_loop != nullptr);

    d_->sp_thread_pool = new eventx::ThreadPool(d_->sp_loop);
    assert(d_->sp_thread_pool != nullptr);
}

Context::~Context()
{
    delete d_->sp_thread_pool;
    delete d_->sp_loop;

    delete d_;
}

bool Context::initialize(const Json &cfg)
{
    //!TODO
    return true;
}

void Context::cleanup()
{
    //!TODO
}

event::Loop* Context::loop() const
{
    return d_->sp_loop;
}

eventx::ThreadPool* Context::thread_pool() const
{
    return d_->sp_thread_pool;
}

}
