#ifndef TBOX_THREAD_POOL_H
#define TBOX_THREAD_POOL_H

#include <limits>
#include <functional>
#include <tbox/event/forward.h>
#include <tbox/base/cabinet_token.h>

namespace tbox {
namespace eventx {

/**
 * 线程池类
 */
class ThreadPool {
  public:
    using TaskToken = cabinet::Token;

    /**
     * 构造函数
     *
     * \param main_loop         主线程的Loop对象指针
     */
    explicit ThreadPool(event::Loop *main_loop);
    virtual ~ThreadPool();

    /**
     * 初始化线程池，指定常驻线程数与最大线程数
     *
     * \param min_thread_num    常驻线程数，必须 >=0
     * \param max_thread_num    最大线程数，必须 >= min_thread_num 且 > 0
     *
     * \return bool     是否成功
     */
    bool initialize(size_t min_thread_num = 0, size_t max_thread_num = std::numeric_limits<size_t>::max());

    using NonReturnFunc = std::function<void ()>;

    /**
     * 使用worker线程执行某个函数
     *
     * \param backend_task      让worker线程执行的函数对象
     * \param prio              任务优先级[-2, -1, 0, 1, 2]，越小优先级越高
     *
     * \return TaskToken        任务Token
     */
    TaskToken execute(const NonReturnFunc &backend_task, int prio = 0);

    /**
     * 使用worker线程执行某个函数，并在完成之后在主线程执行指定的回调函数
     *
     * \param backend_task      让worker线程执行的函数对象
     * \param main_cb           任务完成后，由主线程执行的回调函数对象
     * \param prio              任务优先级[-2, -1, 0, 1, 2]，越小优先级越高
     *
     * \return TaskToken        任务Token
     */
    TaskToken execute(const NonReturnFunc &backend_task, const NonReturnFunc &main_cb, int prio = 0);

    /**
     * 取消任务
     *
     * \return task_token       任务Token
     *
     * \return  int     0: 成功
     *                  1: 没有找到该任务（已执行）
     *                  2: 该任务正在执行
     */
    int cancel(TaskToken task_token);

    /**
     * 清理资源，并等待所有的worker线程结束
     */
    void cleanup();

  protected:
    using ThreadToken = cabinet::Token;

    void threadProc(ThreadToken thread_token);
    bool createWorker();

    bool shouldThreadExitWaiting() const;   //! 判定子线程是否需要退出条件变量的wait()函数

    struct Task;
    Task* popOneTask(); //! 取出一个优先级最高的任务

  private:
    struct Data;
    Data *d_ = nullptr;
};

}
}

#endif //TBOX_THREAD_POOL_H
