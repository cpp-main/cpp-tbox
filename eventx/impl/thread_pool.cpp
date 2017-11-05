#include "thread_pool.h"

#include <tbox/base/log.h>
#include <tbox/event/loop.h>

namespace tbox {
namespace eventx {
namespace impl {

ThreadPool::ThreadPool(event::Loop *main_loop) :
    main_loop_(main_loop)
{ }

ThreadPool::~ThreadPool()
{
    if (is_ready_)
        cleanup();
}

bool ThreadPool::initialize(int min_thread_num, int max_thread_num)
{
    if (is_ready_) {
        LogWarn("it has ready, cleanup() first");
        return false;
    }

    if (max_thread_num < 0 || min_thread_num < 0 ||
        (max_thread_num != 0 && min_thread_num > max_thread_num)) {
        LogWarn("max_thread_num or min_thread_num invalid, max:%d, min:%d", max_thread_num, min_thread_num);
        return false;
    }

    {
        std::lock_guard<std::mutex> lg(lock_);
        min_thread_num_ = min_thread_num;
        max_thread_num_ = max_thread_num;

        for (int i = 0; i < min_thread_num; ++i)
            if (!createWorker())
                return false;
    }

    threads_stop_flag_ = false;
    is_ready_ = true;

    return true;
}

int ThreadPool::execute(const NonReturnFunc &backend_task, int prio)
{
    return execute(backend_task, NonReturnFunc(), prio);
}

int ThreadPool::execute(const NonReturnFunc &backend_task, const NonReturnFunc &main_cb, int prio)
{
    if (!is_ready_) {
        LogWarn("need initialize() first");
        return -1;
    }

    if (prio < -2)
        prio = -2;
    else if (prio > 2)
        prio = 2;

    int level = prio + 2;

    int curr_task_id = 0;

    {
        std::lock_guard<std::mutex> lg(lock_);
        curr_task_id = ++task_id_alloc_;
    }

    TaskEvent *item = new TaskEvent(curr_task_id, backend_task, main_cb);
    if (item == nullptr)
        return -1;

    {
        std::lock_guard<std::mutex> lg(lock_);
        undo_tasks_array_.at(level).push_back(item);
        //! 如果空间线程不够，且还可以再创建新的线程
        if (idle_thread_num_ == 0 &&
            (max_thread_num_ == 0 || threads_.size() < max_thread_num_))
            createWorker();
    }

    cond_var_.notify_one();
    LogInfo("task_id:%d", curr_task_id);

    return curr_task_id;
}

/**
 * 返回值如下：
 * 0: 取消成功
 * 1: 没有找到该任务
 * 2: 该任务正在执行
 */
int ThreadPool::cancel(int task_id)
{
    std::lock_guard<std::mutex> lg(lock_);

    //! 如果正在执行
    if (doing_tasks_set_.find(task_id) != doing_tasks_set_.end())
        return 2;   //! 返回正在执行

    //! 从高优先级向低优先级遍历，找出优先级最高的任务
    for (size_t i = 0; i < undo_tasks_array_.size(); ++i) {
        auto &tasks = undo_tasks_array_.at(i);
        if (!tasks.empty()) {
            for (auto iter = tasks.begin(); iter != tasks.end(); ++iter) {
                TaskEvent *item = *iter;
                if (item->task_id == task_id) {
                    tasks.erase(iter);
                    delete item;
                    return 0;   //! 返回取消成功
                }
            }
        }
    }

    return 1;   //! 返回没有找到
}

void ThreadPool::cleanup()
{
    if (!is_ready_)
        return;

    {
        std::lock_guard<std::mutex> lg(lock_);
        //! 清空task中的任务
        for (size_t i = 0; i < undo_tasks_array_.size(); ++i) {
            auto &tasks = undo_tasks_array_.at(i);
            while (!tasks.empty()) {
                TaskEvent *item = tasks.front();
                delete item;
                tasks.pop_front();
            }
        }
    }

    threads_stop_flag_ = true;
    cond_var_.notify_all();

    //! 等待所有的线程退出
    for (auto item : threads_) {
        std::thread *t = item.second;
        t->join();
        delete t;
    }
    threads_.clear();

    is_ready_ = false;
}

void ThreadPool::threadProc(int id)
{
    LogInfo("thread %d start", id);

    while (true) {
        TaskEvent* item = nullptr;
        {
            std::unique_lock<std::mutex> lk(lock_);

            /**
             * 为防止反复创建线程，此处做优化：
             * 如果当前已有空闲的线程在等待，且当前的线程个数已超过长驻线程数，说明线程数据已满足现有要求
             * 则退出当前线程
             */
            if (idle_thread_num_ > 0 && threads_.size() > min_thread_num_) {
                LogDbg("thread %d will exit, no more work.", id);
                //! 则将线程取出来，交给main_loop去join()，然后delete
                auto iter = threads_.find(id);
                if (iter != threads_.end()) {
                    std::thread *t = iter->second;
                    threads_.erase(iter);
                    main_loop_->runInLoop([t]{ t->join(); delete t; });
                }
                break;
            }

            //! 等待任务
            ++idle_thread_num_;
            cond_var_.wait(lk, std::bind(&ThreadPool::shouldThreadExitWaiting, this));
            --idle_thread_num_;

            /**
             * 有两种情况会从 cond_var_.wait() 退出
             * 1. 任务队列中有任务需要执行时
             * 2. 线程池 cleanup() 时要求所有工作线程退出时
             *
             * 所以，下面检查 threads_stop_flag_ 看是不是请求退出
             */
            if (threads_stop_flag_) {
                LogDbg("thread %d will exit, stop flag.", id);
                break;
            }

            item = popOneTask();    //! 从任务队列中取出优先级最高的任务
        }

        //! 后面就是去执行任务，不需要再加锁了
        if (item != nullptr) {
            {
                std::lock_guard<std::mutex> lg(lock_);
                doing_tasks_set_.insert(item->task_id);
            }

            LogDbg("thread %d pick task %d", id, item->task_id);
            item->backend_task();
            main_loop_->runInLoop(item->main_cb);
            LogDbg("thread %d finish task %d", id, item->task_id);

            {
                std::lock_guard<std::mutex> lg(lock_);
                doing_tasks_set_.erase(item->task_id);
            }
            delete item;
        }
    }

    LogInfo("thread %d exit", id);
}

bool ThreadPool::createWorker()
{
    int curr_thread_id = ++thread_id_alloc_;
    std::thread *new_thread = new std::thread(std::bind(&ThreadPool::threadProc, this, curr_thread_id));
    if (new_thread != nullptr) {
        threads_.insert(std::make_pair(curr_thread_id, new_thread));
        return true;

    } else {
        LogErr("new thread fail");
        return false;
    }
}

bool ThreadPool::shouldThreadExitWaiting() const
{
    if (threads_stop_flag_)
        return true;

    for (size_t i = 0; i < undo_tasks_array_.size(); ++i) {
        const auto &tasks = undo_tasks_array_.at(i);
        if (!tasks.empty()) {
            return true;
        }
    }

    return false;
}

TaskEvent* ThreadPool::popOneTask()
{
    //! 从高优先级向低优先级遍历，找出优先级最高的任务
    for (size_t i = 0; i < undo_tasks_array_.size(); ++i) {
        auto &tasks = undo_tasks_array_.at(i);
        if (!tasks.empty()) {
            TaskEvent* ret = tasks.front();
            tasks.pop_front();
            return ret;
        }
    }
    return nullptr;
}

}
}
}
