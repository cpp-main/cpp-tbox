#include "thread_pool.h"
#include "impl/thread_pool.h"

namespace tbox {
namespace eventx {

ThreadPool::ThreadPool(event::Loop *main_loop) :
    impl_(new impl::ThreadPool(main_loop))
{ }

ThreadPool::~ThreadPool()
{
    delete impl_;
}

bool ThreadPool::initialize(int min_thread_num, int max_thread_num)
{
    return impl_->initialize(min_thread_num, max_thread_num);
}

int ThreadPool::execute(const NonReturnFunc &backend_task, int prio)
{
    return impl_->execute(backend_task, prio);
}

int ThreadPool::execute(const NonReturnFunc &backend_task, const NonReturnFunc &main_cb, int prio)
{
    return impl_->execute(backend_task, main_cb, prio);
}

int ThreadPool::cancel(int task_id)
{
    return impl_->cancel(task_id);
}

void ThreadPool::cleanup()
{
    impl_->cleanup();
}

}
}
