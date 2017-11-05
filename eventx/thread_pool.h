#ifndef TBOX_THREAD_POOL_H
#define TBOX_THREAD_POOL_H

#include <functional>
#include <tbox/event/forward.h>

namespace tbox {
namespace eventx {

namespace impl {
    class ThreadPool;
}

/**
 * 线程池类
 */
class ThreadPool {
  public:
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
     * \param max_thread_num    最大线程数，必须 >=0，=0表示不限定最大线程数量
     *
     * \return bool     是否成功
     */
    bool initialize(int min_thread_num = 0, int max_thread_num = 0);

    using NonReturnFunc = std::function<void ()>;

    /**
     * 使用worker线程执行某个函数
     *
     * \param backend_task      让worker线程执行的函数对象
     * \param prio              任务优先级[-2, -1, 0, 1, 2]，越小优先级越高
     *
     * \return int  <0 任务创建没成功
     *              >=0 任务id
     */
    int execute(const NonReturnFunc &backend_task, int prio = 0);

    /**
     * 使用worker线程执行某个函数，并在完成之后在主线程执行指定的回调函数
     *
     * \param backend_task      让worker线程执行的函数对象
     * \param main_cb           任务完成后，由主线程执行的回调函数对象
     * \param prio              任务优先级[-2, -1, 0, 1, 2]，越小优先级越高
     *
     * \return int  <0 任务创建没成功
     *              >=0 任务id
     */
    int execute(const NonReturnFunc &backend_task, const NonReturnFunc &main_cb, int prio = 0);

    /**
     * 取消任务
     *
     * \param task_id   任务id
     *
     * \return  int     0: 成功
     *                  1: 没有找到该任务（已执行）
     *                  2: 该任务正在执行
     */
    int cancel(int task_id);

    /**
     * 清理资源，并等待所有的worker线程结束
     */
    void cleanup();

  private:
    impl::ThreadPool *impl_;
};

}
}

#endif //TBOX_THREAD_POOL_H
