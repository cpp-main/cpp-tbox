#ifndef TBOX_COROUTINE_SCHEDULER_H_20180519
#define TBOX_COROUTINE_SCHEDULER_H_20180519

#include <queue>
#include <ucontext.h>
#include <tbox/base/defines.h>
#include <tbox/event/loop.h>
#include <tbox/base/cabinet.hpp>

namespace tbox {
namespace coroutine {

class Routine;
class Scheduler;

using RoutineCabinet = Cabinet<Routine>;
using RoutineToken  = RoutineCabinet::Token;
using RoutineEntry  = std::function<void(Scheduler&)>;

class Scheduler {
    friend struct Routine;

  public:
    explicit Scheduler(event::Loop *wp_loop);
    virtual ~Scheduler();

    NONCOPYABLE(Scheduler);
    IMMOVABLE(Scheduler);

  public:
    //! 创建一个协程，并返回协程Token。创建后不自动执行，需要一次 resume()
    RoutineToken create(const RoutineEntry &entry, const std::string &name = "", size_t stack_size = 8192);
    bool resume(const RoutineToken &token); //! 恢复指定协程
    bool cancel(const RoutineToken &token); //! 取消指定协程

  public:
    //! 以下仅限子协程调用
    void wait();    //! 切换到主协程，等待被 resumeRoutine() 唤醒
    void yield();   //! 切换到主协程，等待下一个事件循环继续执行

    RoutineToken getToken() const;  //! 获取当前协程token
    bool isCanceled() const;        //! 当前协程是否被取消
    std::string getName() const;    //! 当前协程的名称

    event::Loop* getLoop() const { return wp_loop_; }

  protected:
    void schedule();    //! 调度，依次切换到已就绪的 Routine 去执行，直到没有 Routine 就绪为止

    bool makeRoutineReady(Routine *routine);
    void switchToRoutine(Routine *routine);
    bool isInMainRoutine() const;   //! 是否处于主协程中

  private:
    event::Loop *wp_loop_ = nullptr;

    ucontext_t main_ctx_;   //! 主协程上下文
    RoutineCabinet routine_cabinet_;
    Routine *curr_routine_ = nullptr;       //! 当前协程的 Routine 对象指针，为 nullptr 表示主协程

    using ReadyRoutineQueue = std::queue<RoutineToken>;
    ReadyRoutineQueue ready_routines_;      //! 已就绪的 Routine 链表
};

}
}

#endif //TBOX_COROUTINE_SCHEDULER_H_20180519
