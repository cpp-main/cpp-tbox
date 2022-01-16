#include "context_imp.h"

#include <cassert>

#include <tbox/base/json.hpp>
#include <tbox/base/log.h>

namespace tbox::main {

ContextImp::ContextImp() :
    sp_loop_(event::Loop::New()),
    sp_thread_pool_(new eventx::ThreadPool(sp_loop_))
{
    assert(sp_loop_ != nullptr);
    assert(sp_thread_pool_ != nullptr);
}

ContextImp::~ContextImp()
{
    delete sp_thread_pool_;
    delete sp_loop_;
}

void ContextImp::fillDefaultConfig(Json &cfg) const
{
    cfg["thread_pool"] = R"({"min":1, "max":0})"_json;
}

bool ContextImp::initialize(const Json &cfg)
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

    if (!sp_thread_pool_->initialize(js_thread_pool_min.get<int>(), js_thread_pool_max.get<int>()))
        return false;

    return true;
}

void ContextImp::cleanup()
{
    sp_thread_pool_->cleanup();
}

}
