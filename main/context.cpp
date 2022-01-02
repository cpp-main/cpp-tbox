#include "context.h"

#include <cassert>

#include <tbox/base/json.hpp>
#include <tbox/base/log.h>

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

void Context::fillDefaultConfig(Json &cfg) const
{
    cfg["thread_pool"] = R"({"min":1, "max":0})"_json;
}

bool Context::initialize(const Json &cfg)
{
    auto &js_thread_pool = cfg["thread_pool"];
    if (js_thread_pool.is_null()) {
        LogWarn("cfg.thread_pool not found");
        return false;
    }

    auto &js_thread_pool_min = js_thread_pool["min"];
    auto &js_thread_pool_max = js_thread_pool["max"];
    if (!js_thread_pool_min.is_number() || !js_thread_pool_max.is_number()) {
        LogWarn("in cfg.thread_pool, min or max is not number");
        return false;
    }

    if (!d_->sp_thread_pool->initialize(js_thread_pool_min.get<int>(),
                                        js_thread_pool_max.get<int>()))
        return false;

    return true;
}

void Context::cleanup()
{
    d_->sp_thread_pool->cleanup();
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
