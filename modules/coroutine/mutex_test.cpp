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
#include <gtest/gtest.h>
#include "mutex.hpp"
#include <tbox/base/scope_exit.hpp>

using namespace std;
using namespace tbox;
using namespace tbox::event;
using namespace tbox::coroutine;

/**
 * 让两个协程同时操作一个变量，观察有没有出现混乱
 */
TEST(Mutex, TwoRoutineUsingSharedValue)
{
    Loop *sp_loop = event::Loop::New();
    SetScopeExitAction([sp_loop]{ delete sp_loop;});

    Scheduler sch(sp_loop);

    Mutex mutex(sch);
    int shared_var = 1; //! 共享的变量

    auto routine1_entry = [&] (Scheduler &sch) {
        for (int i = 0; i < 10; ++i) {
            mutex.lock();
            shared_var *= 2;
            sch.yield();
            shared_var *= 5;
            sch.yield();
            shared_var /= 10;
            mutex.unlock();
            //cout << "1: " << shared_var << endl;
        }
    };

    auto routine2_entry = [&] (Scheduler &sch) {
        for (int i = 0; i < 10; ++i) {
            mutex.lock();
            shared_var += 4;
            sch.yield();
            shared_var += 3;
            sch.yield();
            shared_var -= 7;
            mutex.unlock();
            //cout << "2: " << shared_var << endl;
        }
    };

    sch.create(routine1_entry);
    sch.create(routine2_entry);

    sp_loop->exitLoop(chrono::milliseconds(100));
    sp_loop->runLoop();

    EXPECT_EQ(shared_var, 1);
}

/**
 * 同一个协程重复加锁，应该不阻塞
 */
TEST(Mutex, DuplicateLock)
{
    Loop *sp_loop = event::Loop::New();
    SetScopeExitAction([sp_loop]{ delete sp_loop;});

    Scheduler sch(sp_loop);

    Mutex mutex(sch);
    int times = 3;
    int count = 0;

    auto routine1_entry = [&] (Scheduler &sch) {
        for (int i = 0; i < times; ++i) {
            mutex.lock();
            ++count;
            (void)sch;
        }
    };

    sch.create(routine1_entry);

    sp_loop->exitLoop(chrono::milliseconds(100));
    sp_loop->runLoop();

    EXPECT_EQ(count, times);
}

/**
 * 非法释放，即伪造解锁其它协程加锁的资源
 */
TEST(Mutex, InvalidUnlock)
{
    Loop *sp_loop = event::Loop::New();
    SetScopeExitAction([sp_loop]{ delete sp_loop;});

    Scheduler sch(sp_loop);

    Mutex mutex(sch);

    bool is_routine2_run = false;

    //! 第一个协程，直接对资源进行加锁，然后就退了
    auto routine1_entry = [&] (Scheduler &sch) {
        mutex.lock();
        (void)sch;
    };
    //! 第二个协程，申请资源得不到，应该阻塞这里了
    auto routine2_entry = [&] (Scheduler &sch) {
        sch.yield();
        mutex.lock();
        if (sch.isCanceled())
            return;
        is_routine2_run = true;
    };
    //! 第三个协程，伪造unlock()操作，试图让协程2继续下去
    auto routine3_entry = [&] (Scheduler &sch) {
        sch.yield();
        sch.yield();
        mutex.unlock();
    };


    sch.create(routine1_entry);
    sch.create(routine2_entry);
    sch.create(routine3_entry);

    sp_loop->exitLoop(chrono::milliseconds(100));
    sp_loop->runLoop();

    EXPECT_FALSE(is_routine2_run);
}

/**
 * Mutex::TwoRoutineUsingSharedValue
 * 只不过将 lock() 与 unlock() 操作换成了 Mutex::Locker 执行
 */
TEST(MutexLocker, TwoRoutineUsingSharedValue)
{
    Loop *sp_loop = event::Loop::New();
    SetScopeExitAction([sp_loop]{ delete sp_loop;});

    Scheduler sch(sp_loop);

    Mutex mutex(sch);
    int shared_var = 1; //! 共享的变量

    auto routine1_entry = [&] (Scheduler &sch) {
        for (int i = 0; i < 10; ++i) {
            Mutex::Locker _l(mutex);
            shared_var *= 2;
            sch.yield();
            shared_var *= 5;
            sch.yield();
            shared_var /= 10;
            //cout << "1: " << shared_var << endl;
        }
    };

    auto routine2_entry = [&] (Scheduler &sch) {
        for (int i = 0; i < 10; ++i) {
            Mutex::Locker _l(mutex);
            shared_var += 4;
            sch.yield();
            shared_var += 3;
            sch.yield();
            shared_var -= 7;
            //cout << "2: " << shared_var << endl;
        }
    };

    sch.create(routine1_entry);
    sch.create(routine2_entry);

    sp_loop->exitLoop(chrono::milliseconds(100));
    sp_loop->runLoop();

    EXPECT_EQ(shared_var, 1);

    sch.cleanup();
}

