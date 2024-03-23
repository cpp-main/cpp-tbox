/*
 *     .============.
 *    //  M A K E  / \
 *   //  C++ DEV  /   \
 *  //  E A S Y  /  \/ \
 * ++ ----------.  \/\  .
 *  \\     \     \ /\  /
 *   \\     \     \   /
 *    \\     \     \ /
 *     -============'
 *
 * Copyright (c) 2018 Hevake and contributors, all rights reserved.
 *
 * This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
 * Use of this source code is governed by MIT license that can be found
 * in the LICENSE file in the root of the source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */
#include "scheduler.h"

#include <cstring>

#include <queue>
#include <ucontext.h>

#include <tbox/base/log.h>
#include <tbox/base/assert.h>
#include <tbox/base/cabinet.hpp>

namespace tbox {
namespace coroutine {

using namespace std;
using namespace event;

using RoutineCabinet = cabinet::Cabinet<Routine>;

//! 调度器的数据
struct Scheduler::Data {
    event::Loop *wp_loop = nullptr;

    ucontext_t main_ctx;   //! 主协程上下文
    RoutineCabinet routine_cabinet;
    Routine *curr_routine = nullptr;    //! 当前协程的 Routine 对象指针，为 nullptr 表示主协程

    using ReadyRoutineQueue = std::queue<RoutineToken>;
    ReadyRoutineQueue ready_routines;      //! 已就绪的 Routine 链表
};

//! 协程对象
struct Routine {
    RoutineToken token; //! 协程索引
    RoutineEntry entry; //! 协程函数入口
    string       name;  //! 协程名
    Scheduler   &scheduler; //! 协度器引用

    ucontext_t   ctx;   //! 协程上下文

    //! 协程状态
    enum class State {
        kSuspend,   //!< 挂起，等待被resume()
        kReady,     //!< 就绪
        kRunning,   //!< 正在运行
        kDead,      //!< 已死亡
    };
    State state = State::kSuspend;
    bool is_started  = false;
    bool is_canceled = false;
    RoutineToken join_token;    //! 等待其结束的协程

    void mainEntry()
    {
        LogDbg("Routine %u:%s start", token.id(), name.c_str());
        is_started = true;
        entry(scheduler);
        state = State::kDead;
        LogDbg("Routine %u:%s end", token.id(), name.c_str());
    }

    static void RoutineMainEntry(Routine *p_routine)
    {
        p_routine->mainEntry();
    }

    Routine(const RoutineEntry &e, const string &n, size_t ss, Scheduler &sch) :
        entry(e), name(n), scheduler(sch)
    {
        LogDbg("Routine(%u)", token.id());

        void *p_stack_mem = malloc(ss);
        TBOX_ASSERT(p_stack_mem != nullptr);

        //!TODO: 是否可以加哨兵标记，检查栈溢出

        getcontext(&ctx);
        ctx.uc_stack.ss_size = ss;
        ctx.uc_stack.ss_sp = p_stack_mem;
        ctx.uc_link = &(scheduler.d_->main_ctx);
        makecontext(&ctx, (void(*)(void))RoutineMainEntry, 1, this);
    }

    ~Routine()
    {
        //! 只有没有启动或是已结束的协程才能被释放
        TBOX_ASSERT(!is_started || state == State::kDead);

        free(ctx.uc_stack.ss_sp);
        LogDbg("~Routine(%u)", token.id());
    }
};

///////////////////////////////////////////////////////////////////////////////

Scheduler::Scheduler(Loop *wp_loop) :
    d_(new Data)
{
    TBOX_ASSERT(d_ != nullptr);
    d_->wp_loop = wp_loop;

    ::memset(&d_->main_ctx, 0, sizeof(ucontext_t));
}

Scheduler::~Scheduler()
{
    cleanup();
    delete d_;
}

RoutineToken Scheduler::create(const RoutineEntry &entry, bool run_now, const string &name, size_t stack_size)
{
    Routine *new_routine = new Routine(entry, name, stack_size, *this);
    RoutineToken token = d_->routine_cabinet.alloc(new_routine);
    new_routine->token = token;
    if (run_now)
        makeRoutineReady(new_routine);
    return token;
}

bool Scheduler::resume(const RoutineToken &token)
{
    Routine *routine = d_->routine_cabinet.at(token);
    if (routine != nullptr)
        return makeRoutineReady(routine);
    return false;
}

bool Scheduler::cancel(const RoutineToken &token)
{
    Routine *routine = d_->routine_cabinet.at(token);
    if (routine != nullptr) {
        routine->is_canceled = true;
        return makeRoutineReady(routine);
    }
    return false;
}

void Scheduler::cleanup()
{
    TBOX_ASSERT(isInMainRoutine());  //! 仅限主协程使用

    //! 遍历所有协程，如果未启动的协程则直接删除，如果已启动则标记取消
    d_->routine_cabinet.foreach(
        [this] (Routine *routine) {
            if (!routine->is_started) {
                d_->routine_cabinet.free(routine->token);
                delete routine;
            } else {
                routine->is_canceled = true;
            }
        }
    );

    //! 令已启动的协程尽快正常退出
    while (!d_->routine_cabinet.empty()) {
        d_->routine_cabinet.foreach(
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

    d_->routine_cabinet.clear();
}

void Scheduler::wait()
{
    TBOX_ASSERT(!isInMainRoutine());
    if (d_->curr_routine->is_canceled)
        return;

    d_->curr_routine->state = Routine::State::kSuspend;
    swapcontext(&(d_->curr_routine->ctx), &d_->main_ctx);
}

void Scheduler::yield()
{
    TBOX_ASSERT(!isInMainRoutine());
    if (d_->curr_routine->is_canceled)
        return;

    makeRoutineReady(d_->curr_routine);
    swapcontext(&(d_->curr_routine->ctx), &d_->main_ctx);
}

bool Scheduler::join(const RoutineToken &other_routine)
{
    TBOX_ASSERT(!isInMainRoutine());
    if (d_->curr_routine->is_canceled)
        return false;

    Routine *routine = d_->routine_cabinet.at(other_routine);
    if (routine != nullptr) {
        //! 如果已经结束了的，就直接返回成功
        if (routine->state == Routine::State::kDead)
            return true;
        //! 如果已被其它协程join()了的，就返回失败
        if (!routine->join_token.isNull())
            return false;

        routine->join_token = d_->curr_routine->token;

        d_->curr_routine->state = Routine::State::kSuspend;
        swapcontext(&(d_->curr_routine->ctx), &d_->main_ctx);

        //! 如果不是被cancel唤醒的，那返回成功；否则返回失败
        return !d_->curr_routine->is_canceled;
    }
    return false;
}

RoutineToken Scheduler::getToken() const
{
    TBOX_ASSERT(!isInMainRoutine());
    return d_->curr_routine->token;
}

bool Scheduler::isCanceled() const
{
    TBOX_ASSERT(!isInMainRoutine());
    return d_->curr_routine->is_canceled;
}

string Scheduler::getName() const
{
    TBOX_ASSERT(!isInMainRoutine());
    return d_->curr_routine->name;
}

event::Loop* Scheduler::getLoop() const
{
    return d_->wp_loop;
}

/**
 * 将 routine 状态置为 kReady，然后将其丢到就绪列表中
 */
bool Scheduler::makeRoutineReady(Routine *routine)
{
    if (routine->state == Routine::State::kReady
        || routine->state == Routine::State::kDead)
        return false;

    routine->state = Routine::State::kReady;
    d_->ready_routines.push(routine->token);
    d_->wp_loop->runNext(std::bind(&Scheduler::schedule, this), "Scheduler::makeRoutineReady");
    return true;
}

void Scheduler::switchToRoutine(Routine *routine)
{
    TBOX_ASSERT(isInMainRoutine());

    d_->curr_routine = routine;
    d_->curr_routine->state = Routine::State::kRunning;

    //! 切换到 curr_routine 指定协程去执行
    swapcontext(&d_->main_ctx, &(d_->curr_routine->ctx));
    //! 从 curr_routine 指定协程返回来

    //! 检查协程状态，如果已经结束了的协程，要释放资源
    if (routine->state == Routine::State::kDead) {
        d_->routine_cabinet.free(routine->token);

        //! 如果有其它协程在join这个协程，那么要唤醒等待的协程
        if (!d_->curr_routine->join_token.isNull())
            resume(d_->curr_routine->join_token);

        delete d_->curr_routine;
    }

    d_->curr_routine = nullptr;
}

void Scheduler::schedule()
{
    TBOX_ASSERT(isInMainRoutine());

    //! 思考：为什么要定义一个 tmp 来与 ready_routines 进行交换，而不是直接使用？
    //!
    //! 如果存在某个协程，在循环里反复 yield()，则会导致 ready_routines 一直不为空。
    //! 进而导致 schedule() 函数无法退出。导致 Loop 中的其它事件阻塞，得不到处理。
    //! 这里的处理方法，就是将 ready_routines 中的内容移到 tmp 中来。后来执行中就绪
    //! 的协程留到下一轮去处理。
    Data::ReadyRoutineQueue tmp;
    std::swap(tmp, d_->ready_routines);

    //! 逐一切换到就绪链表对应的协程去执行，直到就绪链表为空
    while (!tmp.empty()) {
        Routine *routine = d_->routine_cabinet.at(tmp.front());
        if (routine != nullptr)
            switchToRoutine(routine);
        tmp.pop();
    }
}

bool Scheduler::isInMainRoutine() const
{
    return d_->curr_routine == nullptr;
}

}
}
