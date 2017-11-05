#ifndef TBOX_THREAD_POOL_IMP_H
#define TBOX_THREAD_POOL_IMP_H

#include <array>
#include <map>
#include <set>
#include <list>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>

#include <tbox/event/forward.h>

namespace tbox {
namespace eventx {
namespace impl {

using NonReturnFunc = std::function<void ()>;

/**
 * 任务项
 */
struct TaskEvent {
  int task_id;  //! 任务号
  NonReturnFunc backend_task;   //! 任务在工作线程中执行函数
  NonReturnFunc main_cb;        //! 任务执行完成后由main_loop执行的回调函数

  TaskEvent(int id, const NonReturnFunc &task, const NonReturnFunc &cb) :
      task_id(id), backend_task(task), main_cb(cb)
  { }
};

/**
 * 线程池实现
 */
class ThreadPool {
  public:
    explicit ThreadPool(event::Loop *main_loop);
    virtual ~ThreadPool();

    bool initialize(int min_thread_num, int max_thread_num);

    int execute(const NonReturnFunc &backend_task, int prio);
    int execute(const NonReturnFunc &backend_task, const NonReturnFunc &main_cb, int prio);

    int cancel(int task_id);

    void cleanup();

  protected:
    void threadProc(int id);
    bool createWorker();

    bool shouldThreadExitWaiting() const;   //! 判定子线程是否需要退出条件变量的wait()函数
    TaskEvent* popOneTask(); //! 取出一个优先级最高的任务

  private:
    event::Loop *main_loop_ = nullptr; //!< 主线程

    bool is_ready_ = false;     //! 是否已经初始化了

    size_t min_thread_num_ = 0; //!< 最少的线程个数
    size_t max_thread_num_ = 0; //!< 最多的线程个数

    std::mutex lock_;                //!< 互斥锁
    std::condition_variable cond_var_;   //!< 条件变量

    std::array<std::list<TaskEvent*>, 5> undo_tasks_array_;    //!< 优先级任务列表，5级
    std::set<int/*task_id*/> doing_tasks_set_;   //!< 记录正在从事的任务

    size_t idle_thread_num_ = 0;    //!< 空间线程个数
    std::map<int/*thread_id*/, std::thread*> threads_;    //!< 线程对象
    int thread_id_alloc_ = 0;       //!< 工作线程的ID分配器
    bool threads_stop_flag_ = false;//!< 是否所有工作线程立即停止标记

    int task_id_alloc_ = 0;         //!< 任务id分配计数器
};

}
}
}

#endif //TBOX_THREAD_POOL_IMP_H
