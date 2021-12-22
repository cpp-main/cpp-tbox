#include "context.h"

#include <cassert>

namespace tbox {
namespace main {

Context::Context() :
    sp_loop_(event::Loop::New()),
    sp_thread_pool_(new eventx::ThreadPool(sp_loop_))
{
    assert(sp_loop_ != nullptr);
    assert(sp_thread_pool_ != nullptr);
}

Context::~Context()
{
    delete sp_thread_pool_;
    delete sp_loop_;
}

}
}
