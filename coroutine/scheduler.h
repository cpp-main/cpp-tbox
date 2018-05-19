#ifndef TBOX_COROUTINE_SCHEDULER_H_20180519
#define TBOX_COROUTINE_SCHEDULER_H_20180519

#include <ucontext.h>
#include <tbox/base/defines.h>
#include <tbox/event/loop.h>
#include <tbox/base/object_locker.hpp>

namespace tbox {
namespace coroutine {

class Routine;

using RoutineLocker = ObjectLocker<Routine>;
using CoId = RoutineLocker::Key;

class Scheduler {
    friend struct Routine;

  public:
    explicit Scheduler(event::Loop *wp_loop);
    virtual ~Scheduler();

  public:
    //! 创建一个协程
    CoId createCo(RoutineEntry entry, const std::string &name = "", size_t stack_size = 8192);
    bool resumeCo(CoId id);    //! 恢复协程
    bool cancelCo(CoId id);    //! 取消协程

  public:
    void wait();    //! 切换到主协程，等待被 resumeCo() 唤醒
    void yield();   //! 切换到主协程，等待下一个事件循环继续执行

    CoId getCoId() const;           //! 获取当前协程id
    bool isCanceled() const;        //! 当前协程是否被取消
    std::string getName() const;    //! 当前协程的名称

    event::Loop getLoop() const { return wp_loop_; }

  public:
    //! 主协程调用
    void schedule();    //! 调度，依次切换到就绪的 Routine 去执行，直到没有 Routine 就绪为止

  protected:
    bool makeRoutineReady(Routine *routine);
    void swapToRoutine(Routine *routine);
    bool isInMainRoutine() const;   //! 是否处于主协程中

  private:
    event::Loop *wp_loop_ = nullptr;
    ucontext_t main_ctx_;   //! 主协程上下文
    RoutineLocker routine_locker_;
    Routine *curr_routine_ = nullptr;   //! 当前协程的 Routine 对象指针，为 nullptr 表示主协程
    std::list<Routine *> lst_read_routines_;    //! 已就绪的 Routine 链表
};

}
}

#endif //TBOX_COROUTINE_SCHEDULER_H_20180519
