#include "scheduler.h"
#include <tbox/base/log.h>
#include <cassert>
#include <cstring>

namespace tbox {
namespace coroutine {

using namespace std;
using namespace event;

struct Routine {
    RoutineKey   key;   //! 协程索引
    RoutineEntry entry; //! 协程函数入口
    string       name;  //! 协程名
    Scheduler   &scheduler; //! 协度器引用

    ucontext_t   ctx;   //! 协程上下文

    //! 协程状态
    enum class State {
        kNotStart,  //!< 未启动
        kSuspend,   //!< 挂起，等待被resume()
        kReady,     //!< 就绪
        kRunning,   //!< 正在运行
        kDead,      //!< 已死亡
    };
    State state = State::kNotStart;
    bool is_canceled = false;

    void mainEntry()
    {
        LogDbg("Routine %u:%s start", key.getId(), name.c_str());
        entry(scheduler);
        state = State::kDead;
        LogDbg("Routine %u:%s end", key.getId(), name.c_str());
    }

    static void RoutineMainEntry(Routine *p_routine)
    {
        p_routine->mainEntry();
    }

    Routine(const RoutineEntry &e, const string &n, size_t ss, Scheduler &sch) :
        entry(e), name(n), scheduler(sch)
    {
        LogDbg("Routine(%u)", key.getId());

        void *p_stack_mem = malloc(ss);
        assert(p_stack_mem != nullptr);

        getcontext(&ctx);
        ctx.uc_stack.ss_size = ss;
        ctx.uc_stack.ss_sp = p_stack_mem;
        ctx.uc_link = &(scheduler.main_ctx_);
        makecontext(&ctx, (void(*)(void))RoutineMainEntry, 1, this);
    }

    ~Routine()
    {
        assert(state == State::kNotStart || state == State::kDead);

        free(ctx.uc_stack.ss_sp);
        LogDbg("~Routine(%u)", key.getId());
    }
};

///////////////////////////////////////////////////////////////////////////////

Scheduler::Scheduler(Loop *wp_loop) :
    wp_loop_(wp_loop)
{
    ::memset(&main_ctx_, 0, sizeof(main_ctx_));
}

Scheduler::~Scheduler()
{
    //! 遍历所有协程，如果未启动的协程则直接删除，如果已启动则标记取消
    routine_locker_.foreach(
        [this] (Routine *routine) {
            if (routine->state == Routine::State::kNotStart) {
                routine_locker_.remove(routine->key);
                delete routine;
            } else {
                routine->is_canceled = true;
            }
        }
    );

    //! 令已启动的协程尽快正常退出
    while (!routine_locker_.empty()) {
        routine_locker_.foreach(
            [this] (Routine *routine) {
                switchToRoutine(routine);
            }
        );
    }
    //! 思考：为什么不能直接删除已启动的协程？
    //!
    //! 因为直接删除很可能会引起协程的资源得不到释放，比如对象的析构函数被跳过。
    //! 轻则引起内存泄漏，重则阻塞、崩溃。
    //! 所以，这里的解决办法：令协程自行了结。
}

RoutineKey Scheduler::create(const RoutineEntry &entry, const string &name, size_t stack_size)
{
    Routine *new_routine = new Routine(entry, name, stack_size, *this);
    RoutineKey key = routine_locker_.insert(new_routine);
    new_routine->key = key;
    return key;
}

bool Scheduler::resume(const RoutineKey &key)
{
    Routine *routine = routine_locker_.at(key);
    if (routine != nullptr)
        return makeRoutineReady(routine);
    return false;
}

bool Scheduler::cancel(const RoutineKey &key)
{
    Routine *routine = routine_locker_.at(key);
    if (routine != nullptr) {
        routine->is_canceled = true;
        return makeRoutineReady(routine);
    }
    return false;
}

void Scheduler::wait()
{
    assert(!isInMainRoutine());
    if (curr_routine_->is_canceled)
        return;

    curr_routine_->state = Routine::State::kSuspend;
    swapcontext(&(curr_routine_->ctx), &main_ctx_);
}

void Scheduler::yield()
{
    assert(!isInMainRoutine());
    if (curr_routine_->is_canceled)
        return;

    makeRoutineReady(curr_routine_);
    swapcontext(&(curr_routine_->ctx), &main_ctx_);
}

RoutineKey Scheduler::getKey() const
{
    assert(!isInMainRoutine());
    return curr_routine_->key;
}

bool Scheduler::isCanceled() const
{
    assert(!isInMainRoutine());
    return curr_routine_->is_canceled;
}

string Scheduler::getName() const
{
    assert(!isInMainRoutine());
    return curr_routine_->name;
}

/**
 * 将 routine 状态置为 kReady，然后将其丢到就绪列表中
 */
bool Scheduler::makeRoutineReady(Routine *routine)
{
    if ((routine->state == Routine::State::kReady)
        || (routine->state == Routine::State::kDead))
        return false;

    routine->state = Routine::State::kReady;
    ready_routines_.push(routine);
    wp_loop_->runInLoop(std::bind(&Scheduler::schedule, this));
    return true;
}

void Scheduler::switchToRoutine(Routine *routine)
{
    assert(isInMainRoutine());

    curr_routine_ = routine;
    curr_routine_->state = Routine::State::kRunning;

    //! 切换到 curr_routine_ 指定协程去执行
    swapcontext(&main_ctx_, &(curr_routine_->ctx));
    //! 从 curr_routine_ 指定协程返回来

    //! 检查协程状态，如果已经结束了的协程，要释放资源
    if (routine->state == Routine::State::kDead) {
        routine_locker_.remove(routine->key);
        delete curr_routine_;
    }

    curr_routine_ = nullptr;
}

void Scheduler::schedule()
{
    assert(isInMainRoutine());

    //! 逐一切换到就绪链表对应的协程去执行，直到就绪链表为空
    while (!ready_routines_.empty()) {
        Routine *routine = ready_routines_.front();
        switchToRoutine(routine);
        ready_routines_.pop();
    }
}

bool Scheduler::isInMainRoutine() const
{
    return curr_routine_ == nullptr;
}

}
}
