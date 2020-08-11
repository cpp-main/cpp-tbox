#include "thread_pool.h"

#include <array>
#include <map>
#include <set>
#include <deque>
#include <thread>
#include <mutex>
#include <condition_variable>

#include <tbox/base/log.h>
#include <tbox/event/loop.h>

namespace tbox {
namespace eventx {

//! ThreadPool 的私有数据
struct ThreadPool::Data {
    event::Loop *wp_loop = nullptr; //!< 主线程

    bool is_ready = false;     //! 是否已经初始化了

    size_t min_thread_num = 0; //!< 最少的线程个数
    size_t max_thread_num = 0; //!< 最多的线程个数

    std::mutex lock;                //!< 互斥锁
    std::condition_variable cond_var;   //!< 条件变量

    std::array<std::deque<Task*>, 5> undo_tasks; //!< 优先级任务列表，5级
    std::set<int/*task_id*/> doing_tasks_set;    //!< 记录正在从事的任务

    size_t idle_thread_num = 0;         //!< 空间线程个数
    std::map<int/*thread_id*/, std::thread*> threads;    //!< 线程对象
    int thread_id_alloc = 0;            //!< 工作线程的ID分配器
    bool all_threads_stop_flag = false; //!< 是否所有工作线程立即停止标记

    int task_id_alloc = 0;             //!< 任务id分配计数器
};

/**
 * 任务项
 */
struct ThreadPool::Task {
  int task_id;  //! 任务号
  NonReturnFunc backend_task;   //! 任务在工作线程中执行函数
  NonReturnFunc main_cb;        //! 任务执行完成后由main_loop执行的回调函数

  Task(int id, const NonReturnFunc &task, const NonReturnFunc &cb) :
      task_id(id), backend_task(task), main_cb(cb)
  { }
};

/////////////////////////////////////////////////////////////////////////////////

ThreadPool::ThreadPool(event::Loop *main_loop) :
    d_(new Data)
{
    d_->wp_loop = main_loop;
}

ThreadPool::~ThreadPool()
{
    if (d_->is_ready)
        cleanup();

    delete d_;
}

bool ThreadPool::initialize(int min_thread_num, int max_thread_num)
{
    if (d_->is_ready) {
        LogWarn("it has ready, cleanup() first");
        return false;
    }

    if (max_thread_num < 0 || min_thread_num < 0 ||
        (max_thread_num != 0 && min_thread_num > max_thread_num)) {
        LogWarn("max_thread_num or min_thread_num invalid, max:%d, min:%d", max_thread_num, min_thread_num);
        return false;
    }

    {
        std::lock_guard<std::mutex> lg(d_->lock);
        d_->min_thread_num = min_thread_num;
        d_->max_thread_num = max_thread_num;

        for (int i = 0; i < min_thread_num; ++i)
            if (!createWorker())
                return false;
    }

    d_->all_threads_stop_flag = false;
    d_->is_ready = true;

    return true;
}

int ThreadPool::execute(const NonReturnFunc &backend_task, int prio)
{
    return execute(backend_task, NonReturnFunc(), prio);
}

int ThreadPool::execute(const NonReturnFunc &backend_task, const NonReturnFunc &main_cb, int prio)
{
    if (!d_->is_ready) {
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
        std::lock_guard<std::mutex> lg(d_->lock);
        curr_task_id = ++d_->task_id_alloc;
    }

    Task *item = new Task(curr_task_id, backend_task, main_cb);
    if (item == nullptr)
        return -1;

    {
        std::lock_guard<std::mutex> lg(d_->lock);
        d_->undo_tasks.at(level).push_back(item);
        //! 如果空间线程不够，且还可以再创建新的线程
        if (d_->idle_thread_num == 0 &&
            (d_->max_thread_num == 0 || d_->threads.size() < d_->max_thread_num))
            createWorker();
    }

    d_->cond_var.notify_one();
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
    std::lock_guard<std::mutex> lg(d_->lock);

    //! 如果正在执行
    if (d_->doing_tasks_set.find(task_id) != d_->doing_tasks_set.end())
        return 2;   //! 返回正在执行

    //! 从高优先级向低优先级遍历，找出优先级最高的任务
    for (size_t i = 0; i < d_->undo_tasks.size(); ++i) {
        auto &tasks = d_->undo_tasks.at(i);
        if (!tasks.empty()) {
            for (auto iter = tasks.begin(); iter != tasks.end(); ++iter) {
                Task *item = *iter;
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
    if (!d_->is_ready)
        return;

    {
        std::lock_guard<std::mutex> lg(d_->lock);
        //! 清空task中的任务
        for (size_t i = 0; i < d_->undo_tasks.size(); ++i) {
            auto &tasks = d_->undo_tasks.at(i);
            while (!tasks.empty()) {
                Task *item = tasks.front();
                delete item;
                tasks.pop_front();
            }
        }
    }

    d_->all_threads_stop_flag = true;
    d_->cond_var.notify_all();

    //! 等待所有的线程退出
    for (auto item : d_->threads) {
        std::thread *t = item.second;
        t->join();
        delete t;
    }
    d_->threads.clear();
    d_->is_ready = false;
}

void ThreadPool::threadProc(int id)
{
    LogInfo("thread %d start", id);

    while (true) {
        Task* item = nullptr;
        {
            std::unique_lock<std::mutex> lk(d_->lock);

            /**
             * 为防止反复创建线程，此处做优化：
             * 如果当前已有空闲的线程在等待，且当前的线程个数已超过长驻线程数，说明线程数据已满足现有要求
             * 则退出当前线程
             */
            if (d_->idle_thread_num > 0 && d_->threads.size() > d_->min_thread_num) {
                LogDbg("thread %d will exit, no more work.", id);
                //! 则将线程取出来，交给main_loop去join()，然后delete
                auto iter = d_->threads.find(id);
                if (iter != d_->threads.end()) {
                    std::thread *t = iter->second;
                    d_->threads.erase(iter);
                    d_->wp_loop->runInLoop([t]{ t->join(); delete t; });
                }
                break;
            }

            //! 等待任务
            ++d_->idle_thread_num;
            d_->cond_var.wait(lk, std::bind(&ThreadPool::shouldThreadExitWaiting, this));
            --d_->idle_thread_num;

            /**
             * 有两种情况会从 cond_var.wait() 退出
             * 1. 任务队列中有任务需要执行时
             * 2. 线程池 cleanup() 时要求所有工作线程退出时
             *
             * 所以，下面检查 all_threads_stop_flag 看是不是请求退出
             */
            if (d_->all_threads_stop_flag) {
                LogDbg("thread %d will exit, stop flag.", id);
                break;
            }

            item = popOneTask();    //! 从任务队列中取出优先级最高的任务
        }

        //! 后面就是去执行任务，不需要再加锁了
        if (item != nullptr) {
            {
                std::lock_guard<std::mutex> lg(d_->lock);
                d_->doing_tasks_set.insert(item->task_id);
            }

            LogDbg("thread %d pick task %d", id, item->task_id);
            item->backend_task();
            d_->wp_loop->runInLoop(item->main_cb);
            LogDbg("thread %d finish task %d", id, item->task_id);

            {
                std::lock_guard<std::mutex> lg(d_->lock);
                d_->doing_tasks_set.erase(item->task_id);
            }
            delete item;
        }
    }

    LogInfo("thread %d exit", id);
}

bool ThreadPool::createWorker()
{
    int curr_thread_id = ++d_->thread_id_alloc;
    std::thread *new_thread = new std::thread(std::bind(&ThreadPool::threadProc, this, curr_thread_id));
    if (new_thread != nullptr) {
        d_->threads.insert(std::make_pair(curr_thread_id, new_thread));
        return true;

    } else {
        LogErr("new thread fail");
        return false;
    }
}

bool ThreadPool::shouldThreadExitWaiting() const
{
    if (d_->all_threads_stop_flag)
        return true;

    for (size_t i = 0; i < d_->undo_tasks.size(); ++i) {
        const auto &tasks = d_->undo_tasks.at(i);
        if (!tasks.empty()) {
            return true;
        }
    }

    return false;
}

ThreadPool::Task* ThreadPool::popOneTask()
{
    //! 从高优先级向低优先级遍历，找出优先级最高的任务
    for (size_t i = 0; i < d_->undo_tasks.size(); ++i) {
        auto &tasks = d_->undo_tasks.at(i);
        if (!tasks.empty()) {
            Task* ret = tasks.front();
            tasks.pop_front();
            return ret;
        }
    }
    return nullptr;
}

}
}
