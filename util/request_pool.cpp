#include "request_pool.h"
#include <tbox/base/log.h>

namespace tbox {
namespace util {

RequestPool::RequestPool(event::Loop *wp_loop)
{

}

RequestPool::~RequestPool()
{

}

bool RequestPool::initialize(const Seconds &timeout_sec, const TimeoutAction &timeout_action)
{
    return false;
}

bool RequestPool::start()
{
    return false;
}

void RequestPool::stop()
{

}

void RequestPool::cleanup()
{

}

RequestPool::Token RequestPool::newRequest(void *req_ctx)
{
    return Token();
}

bool RequestPool::updateRequest(const Token &token, void *req_ctx)
{
    return false;
}

void* RequestPool::removeRequest(const Token &token)
{
    return nullptr;
}

}
}
