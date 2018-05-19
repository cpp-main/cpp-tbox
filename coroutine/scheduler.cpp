#include "scheduler.h"
#include <tbox/base/log.h>
#include <cassert>
#include <cstring>

namespace tbox {
namespace coroutine {

using namespace std;
using namespace event;

struct Routine {
    //! 状态
    enum class State {
        kSuspend,   //!< 挂起
        kReady,     //!< 就绪
        kRunning,   //!< 正在运行
        kDead,      //!< 已死亡
    };

    CoId id;
    RoutineEntry entry_func;
    string name;
    Scheduler &scheduler;

    ucontext_t ctx;

    State state = State::kSuspend;
    bool is_canceled = false;
    bool is_started = false;

    void mainEntry()
    {
        LogDbg("Routine %u:%s start", id.getId(), name.c_str());
        is_started = true;
        entry_func(scheduler);
        state = State::kDead;
        LogDbg("Routine %u:%s end", id.getId(), name.c_str());
    }

    static void RoutineMainEntry(Routine *p_routine)
    {
        p_routine->mainEntry();
    }

    Routine(RoutineEntry e, const string &n, size_t ss, Scheduler &sch) :
        entry_func(e), name(n), scheduler(sch)
    {
        LogDbg("Routine(%u)", id.getId());

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
        free(ctx.uc_stack.ss_sp);
        LogDbg("~Routine(%u)", id.getId());
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
    //TODO
}

CoId Scheduler::createCo(RoutineEntry entry, const string &name, size_t stack_size)
{
    Routine *new_routine = new Routine(entry, name, stack_size, *this);
    CoId id = routine_locker_.insert(new_routine);
    new_routine->id = id;
    return id;
}

bool Scheduler::makeRoutineReady(Routine *routine)
{
    if ((routine->state == Routine::State::kReady)
        || (routine->state == Routine::State::kDead))
        return false;

    routine->state = Routine::State::kReady;
    lst_ready_routines_.push_back(routine);

    wp_loop_->runInLoop(std::bind(&Scheduler::schedule, this));
    return true;
}

bool Scheduler::resumeCo(CoId id)
{
    Routine *routine = routine_locker_.at(id);
    if (routine != nullptr)
        return makeRoutineReady(routine);
    return false;
}

bool Scheduler::cancelCo(CoId id)
{
    Routine *routine = routine_locker_.at(id);
    if (routine != nullptr) {
        routine->is_canceled = true;
        return makeRoutineReady(routine);
    }
    return false;
}

void Scheduler::wait()
{
    //!TODO
}

void Scheduler::yield()
{
    //!TODO
}

CoId Scheduler::getCoId() const
{
    assert(!isInMainRoutine());
    return curr_routine_->id;
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

void Scheduler::swapToRoutine(Routine *routine)
{
    //!TODO
}

void Scheduler::schedule()
{
    //!TODO
}

bool Scheduler::isInMainRoutine() const
{
    return curr_routine_ == nullptr;
}

}
}
