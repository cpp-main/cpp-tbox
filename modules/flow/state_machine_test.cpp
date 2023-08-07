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

#include <tbox/base/json.hpp>
#include "state_machine.h"

using namespace std;

namespace tbox {
namespace flow {

using SM = StateMachine;

enum StateId {
    STATE_TERM = 0,
    STATE_A,
    STATE_B,
    STATE_C,
    STATE_D,
};

enum EventId {
    EVENT_ANY = 0,
    EVENT_1,
    EVENT_2,
    EVENT_3,
    EVENT_4,
};

//! 测试创建状态
TEST(StateMachine, NewState)
{
    SM sm;
    EXPECT_TRUE (sm.newState(STATE_A, nullptr, nullptr));
    EXPECT_TRUE (sm.newState(STATE_B, nullptr, nullptr));
    EXPECT_FALSE(sm.newState(STATE_A, nullptr, nullptr));
}

//! 测试创建事件
TEST(StateMachine, AddRoute)
{
    SM sm;
    sm.newState(STATE_A, nullptr, nullptr);
    sm.newState(STATE_B, nullptr, nullptr);

    EXPECT_TRUE (sm.addRoute(STATE_A, EVENT_1, STATE_B, nullptr, nullptr));
    EXPECT_FALSE(sm.addRoute(STATE_A, EVENT_1, STATE_C, nullptr, nullptr));
    EXPECT_FALSE(sm.addRoute(STATE_C, EVENT_1, STATE_B, nullptr, nullptr));
    EXPECT_FALSE(sm.addRoute(STATE_C, EVENT_1, STATE_D, nullptr, nullptr));
}

//! 测试创建两个状态，一个事件由A到B转换，没有守卫与执行函数
TEST(StateMachine, StateWithoutGuardAndAction)
{
    SM sm;
    sm.newState(STATE_A, nullptr, nullptr);
    sm.newState(STATE_B, nullptr, nullptr);
    sm.addRoute(STATE_A, EVENT_1, STATE_B, nullptr, nullptr);
    sm.setInitState(STATE_A);
    sm.start();

    EXPECT_EQ(sm.currentState(), STATE_A);
    EXPECT_TRUE(sm.run(EVENT_1));
    EXPECT_EQ(sm.currentState(), STATE_B);
    EXPECT_FALSE(sm.run(EVENT_2));
}

//! 测试创建两个状态，一个事件由A到B转换，有守卫
TEST(StateMachine, RouteWithGuard)
{
    SM sm;
    sm.newState(STATE_A, nullptr, nullptr);
    sm.newState(STATE_B, nullptr, nullptr);
    sm.newState(STATE_C, nullptr, nullptr);

    bool condition = false;
    sm.addRoute(STATE_A, EVENT_1, STATE_B, [&](Event){ return !condition; }, nullptr);
    sm.addRoute(STATE_A, EVENT_1, STATE_C, [&](Event){ return condition; },  nullptr);
    sm.addRoute(STATE_B, EVENT_2, STATE_A, nullptr, nullptr);
    sm.addRoute(STATE_C, EVENT_2, STATE_A, nullptr, nullptr);
    sm.setInitState(STATE_A);
    sm.start();

    EXPECT_EQ(sm.currentState(), STATE_A);

    condition = false;
    EXPECT_TRUE(sm.run(EVENT_1));
    EXPECT_EQ(sm.currentState(), STATE_B);

    EXPECT_TRUE(sm.run(EVENT_2));
    EXPECT_EQ(sm.currentState(), STATE_A);

    condition = true;
    EXPECT_TRUE(sm.run(EVENT_1));
    EXPECT_EQ(sm.currentState(), STATE_C);

    EXPECT_TRUE(sm.run(EVENT_2));
    EXPECT_EQ(sm.currentState(), STATE_A);
}

//! 测试创建两个状态，一个事件由A到B转换，有进入与退出动作
TEST(StateMachine, StateWithEnterAndExitAction)
{
    SM sm;

    int a_enter_counter = 0;
    int a_exit_counter = 0;
    int b_enter_counter = 0;
    int b_exit_counter = 0;

    sm.newState(STATE_A, [&](Event){ ++a_enter_counter; }, [&](Event){ ++a_exit_counter; });
    sm.newState(STATE_B, [&](Event){ ++b_enter_counter; }, [&](Event){ ++b_exit_counter; });

    sm.addRoute(STATE_A, EVENT_1, STATE_B, nullptr, nullptr);
    sm.addRoute(STATE_B, EVENT_2, STATE_A, nullptr, nullptr);

    sm.setInitState(STATE_A);
    EXPECT_EQ(a_enter_counter, 0);
    sm.start();
    EXPECT_EQ(a_enter_counter, 1);

    sm.run(EVENT_1);
    EXPECT_EQ(a_exit_counter, 1);
    EXPECT_EQ(b_enter_counter, 1);

    sm.run(EVENT_2);
    EXPECT_EQ(b_exit_counter, 1);
    EXPECT_EQ(a_enter_counter, 2);
}

//! 测试创建两个状态，一个事件由A到B转换，事件有转换动作
TEST(StateMachine, EventWithAction)
{
    SM sm;

    sm.newState(STATE_A, nullptr, nullptr);
    sm.newState(STATE_B, nullptr, nullptr);

    int count_1 = 0;
    int count_2 = 0;

    sm.addRoute(STATE_A, EVENT_1, STATE_B, nullptr, [&](Event){ ++count_1; });
    sm.addRoute(STATE_B, EVENT_2, STATE_A, nullptr, [&](Event){ ++count_2; });
    sm.setInitState(STATE_A);
    sm.start();

    sm.run(EVENT_1);
    EXPECT_EQ(count_1, 1);

    sm.run(EVENT_2);
    EXPECT_EQ(count_2, 1);
}

TEST(StateMachine, AnyEvent)
{
    SM sm;

    sm.newState(STATE_A, nullptr, nullptr);
    sm.newState(STATE_B, nullptr, nullptr);

    sm.addRoute(STATE_A, 0, STATE_B, nullptr, nullptr);
    sm.addRoute(STATE_B, 0, STATE_A, nullptr, nullptr);
    sm.setInitState(STATE_A);
    sm.start();

    sm.run(EVENT_1);
    EXPECT_EQ(sm.currentState(), STATE_B);
    sm.run(EVENT_2);
    EXPECT_EQ(sm.currentState(), STATE_A);
    sm.run(EVENT_3);
    EXPECT_EQ(sm.currentState(), STATE_B);
}

TEST(StateMachine, Restart)
{
    SM sm;

    int a_enter_counter = 0;
    int a_exit_counter = 0;
    int b_enter_counter = 0;
    int b_exit_counter = 0;

    sm.newState(STATE_A, [&](Event){ ++a_enter_counter; }, [&](Event){ ++a_exit_counter; });
    sm.newState(STATE_B, [&](Event){ ++b_enter_counter; }, [&](Event){ ++b_exit_counter; });

    sm.addRoute(STATE_A, EVENT_1, STATE_B, nullptr, nullptr);
    sm.setInitState(STATE_A);
    sm.start();
    EXPECT_EQ(a_enter_counter, 1);
    EXPECT_TRUE(sm.run(EVENT_1));
    EXPECT_EQ(a_exit_counter, 1);
    EXPECT_EQ(b_enter_counter, 1);
    sm.restart();
    EXPECT_EQ(b_exit_counter, 1);
    EXPECT_EQ(a_enter_counter, 2);
    EXPECT_EQ(sm.currentState(), STATE_A);
}

//! 主要测试用 enum class 定义的状态与事件是否能编译过
TEST(StateMachine, EnumClass)
{
    SM sm;
    enum class StateId { k1 = 1, k2 };
    enum class EventId { k1 = 1, k2 };

    int enter_counter = 0;
    int exit_counter = 0;

    sm.newState(StateId::k1, [&](Event){ ++enter_counter; }, [&](Event){ ++exit_counter; });
    sm.newState(StateId::k2, nullptr, nullptr);

    sm.addRoute(StateId::k1, EventId::k1, StateId::k2, nullptr, nullptr);
    sm.addRoute(StateId::k2, EventId::k2, StateId::k1, nullptr, nullptr);
    sm.setInitState(StateId::k1);
    sm.start();
    sm.run(EventId::k1);
    sm.run(EventId::k2);
    sm.stop();

    EXPECT_EQ(enter_counter, 2);
    EXPECT_EQ(exit_counter, 2);
}

TEST(StateMachine, SubSM)
{
    enum class StateId { kTerm, kInit, k1, k2 };
    enum class EventId { kTerm, k1, k2 };

    SM sm, sub_sm;

    sm.newState(StateId::kInit, nullptr, nullptr);
    sm.newState(StateId::k1,    nullptr, nullptr);
    sm.newState(StateId::k2,    nullptr, nullptr);

    sm.addRoute(StateId::kInit, EventId::k1, StateId::k1,    nullptr, nullptr);
    sm.addRoute(StateId::kInit, EventId::k2, StateId::k2,    nullptr, nullptr);
    sm.addRoute(StateId::k1,    EventId::k1, StateId::kTerm, nullptr, nullptr);
    sm.addRoute(StateId::k1,    EventId::k2, StateId::k2,    nullptr, nullptr);
    sm.addRoute(StateId::k2,    EventId::k1, StateId::k1,    nullptr, nullptr);
    sm.addRoute(StateId::k2,    EventId::k2, StateId::k1,    nullptr, nullptr);

    sub_sm.newState(StateId::kInit, nullptr, nullptr);
    sub_sm.newState(StateId::k1,    nullptr, nullptr);
    sub_sm.newState(StateId::k2,    nullptr, nullptr);

    sub_sm.addRoute(StateId::kInit, EventId::k1, StateId::k1,    nullptr, nullptr);
    sub_sm.addRoute(StateId::kInit, EventId::k2, StateId::k2,    nullptr, nullptr);
    sub_sm.addRoute(StateId::k1,    EventId::k2, StateId::k2,    nullptr, nullptr);
    sub_sm.addRoute(StateId::k1,    EventId::k1, StateId::kTerm, nullptr, nullptr);
    sub_sm.addRoute(StateId::k2,    EventId::k1, StateId::k1,    nullptr, nullptr);
    sub_sm.addRoute(StateId::k2,    EventId::k2, StateId::kTerm, nullptr, nullptr);

    sm.setSubStateMachine(StateId::k1, &sub_sm);

    {   //! 直通
        sm.start();

        EXPECT_EQ(sm.currentState<StateId>(), StateId::kInit);
        EXPECT_EQ(sub_sm.currentState(), -1);

        EXPECT_TRUE(sm.run(EventId::k1));
        EXPECT_EQ(sm.currentState<StateId>(), StateId::k1);
        EXPECT_EQ(sub_sm.currentState<StateId>(), StateId::k1);

        EXPECT_TRUE(sm.run(EventId::k1));
        EXPECT_EQ(sm.currentState<StateId>(), StateId::kTerm);
        EXPECT_EQ(sub_sm.currentState(), -1);

        EXPECT_TRUE(sm.isTerminated());

        sm.stop();
    }

    {   //! 在子状态机里转了一下
        sm.start();

        EXPECT_EQ(sm.currentState<StateId>(), StateId::kInit);
        EXPECT_EQ(sub_sm.currentState(), -1);

        EXPECT_TRUE(sm.run(EventId::k1));
        EXPECT_EQ(sm.currentState<StateId>(), StateId::k1);
        EXPECT_EQ(sub_sm.currentState<StateId>(), StateId::k1);

        EXPECT_TRUE(sm.run(EventId::k2));
        EXPECT_EQ(sm.currentState<StateId>(), StateId::k1);
        EXPECT_EQ(sub_sm.currentState<StateId>(), StateId::k2);

        EXPECT_TRUE(sm.run(EventId::k1));
        EXPECT_EQ(sm.currentState<StateId>(), StateId::k1);
        EXPECT_EQ(sub_sm.currentState<StateId>(), StateId::k1);

        EXPECT_TRUE(sm.run(EventId::k1));
        EXPECT_EQ(sm.currentState<StateId>(), StateId::kTerm);
        EXPECT_EQ(sub_sm.currentState(), -1);
        EXPECT_TRUE(sm.isTerminated());

        sm.stop();
    }

    {   //! 从S2进的S1的子状态机，再出来回到S2，再进去
        sm.start();

        EXPECT_EQ(sm.currentState<StateId>(), StateId::kInit);
        EXPECT_EQ(sub_sm.currentState(), -1);

        EXPECT_TRUE(sm.run(EventId::k2));
        EXPECT_EQ(sm.currentState<StateId>(), StateId::k2);
        EXPECT_EQ(sub_sm.currentState(), -1);

        EXPECT_TRUE(sm.run(EventId::k2));
        EXPECT_EQ(sm.currentState<StateId>(), StateId::k1);
        EXPECT_EQ(sub_sm.currentState<StateId>(), StateId::k2);

        EXPECT_TRUE(sm.run(EventId::k2));
        EXPECT_EQ(sm.currentState<StateId>(), StateId::k2);
        EXPECT_EQ(sub_sm.currentState(), -1);

        EXPECT_TRUE(sm.run(EventId::k2));
        EXPECT_EQ(sm.currentState<StateId>(), StateId::k1);
        EXPECT_EQ(sub_sm.currentState<StateId>(), StateId::k2);

        EXPECT_TRUE(sm.run(EventId::k1));
        EXPECT_EQ(sm.currentState<StateId>(), StateId::k1);
        EXPECT_EQ(sub_sm.currentState<StateId>(), StateId::k1);

        EXPECT_TRUE(sm.run(EventId::k1));
        EXPECT_EQ(sm.currentState<StateId>(), StateId::kTerm);
        EXPECT_EQ(sub_sm.currentState(), -1);

        sm.stop();
    }
}

TEST(StateMachine, StateChangedCallback)
{
    enum class StateId { kTerm, kInit, k1 };
    enum class EventId { kAny, k1 };

    SM sm;

    sm.newState(StateId::kInit, nullptr, nullptr);
    sm.newState(StateId::k1,    nullptr, nullptr);

    sm.addRoute(StateId::kInit, EventId::k1, StateId::k1,    nullptr, nullptr);
    sm.addRoute(StateId::k1,    EventId::k1, StateId::kTerm, nullptr, nullptr);

    StateId from, to;
    EventId event;

    sm.setStateChangedCallback(
        [&] (SM::StateID f, SM::StateID t, Event e) {
            from = static_cast<StateId>(f);
            to = static_cast<StateId>(t);
            event = static_cast<EventId>(e.id);
        }
    );
    sm.start();
    sm.run(EventId::k1);
    EXPECT_EQ(from, StateId::kInit);
    EXPECT_EQ(to, StateId::k1);
    EXPECT_EQ(event, EventId::k1);

    sm.run(EventId::k1);
    EXPECT_EQ(from, StateId::k1);
    EXPECT_EQ(to, StateId::kTerm);
    EXPECT_EQ(event, EventId::k1);
}

//! 测试起始状态有子状态的情况
TEST(StateMachine, InitStateHasSubMachine)
{
    enum class StateId { kTerm, kInit };
    enum class EventId { kAny, k1 };

    SM sm;
    SM sub_sm;

    int step = 0;
    sm.newState(StateId::kInit, [&](Event){ EXPECT_EQ(step, 0); ++step; }, [&](Event){ EXPECT_EQ(step, 6); ++step; });
    sm.newState(StateId::kTerm, [&](Event){ EXPECT_EQ(step, 8); ++step; }, [&](Event){ EXPECT_EQ(step, 9); ++step; });
    sm.addRoute(StateId::kInit, EventId::k1, StateId::kTerm, nullptr, [&](Event){ EXPECT_EQ(step, 7); ++step; });

    sub_sm.newState(StateId::kInit, [&](Event){ EXPECT_EQ(step, 1); ++step; }, [&](Event){ EXPECT_EQ(step, 2); ++step; });
    sub_sm.newState(StateId::kTerm, [&](Event){ EXPECT_EQ(step, 4); ++step; }, [&](Event){ EXPECT_EQ(step, 5); ++step; });
    sub_sm.addRoute(StateId::kInit, EventId::k1, StateId::kTerm, nullptr, [&](Event){ EXPECT_EQ(step, 3); ++step; });

    sm.setSubStateMachine(StateId::kInit, &sub_sm);
#if 0
    sm.setStateChangedCallback(
        [&] (SM::StateID f, SM::StateID t, EventID e) {
            cout << f << "-->" << t << "," << e << endl;
        }
    );
    sub_sm.setStateChangedCallback(
        [&] (SM::StateID f, SM::StateID t, EventID e) {
            cout << "  " <<  f << "-->" << t << "," << e << endl;
        }
    );
#endif
    sm.start();
    sm.run(EventId::k1);
    sm.stop();
}

//! 测试各种Action之间的执行顺序
TEST(StateMachine, SubSMActionOrder)
{
    enum class StateId { kTerm, kInit, k1};
    enum class EventId { kAny, k1 };

    SM sm;
    SM sub_sm;

    int step = 0;
    sm.newState(StateId::kInit, [&](Event){ EXPECT_EQ(step, 0); ++step; }, [&](Event){ EXPECT_EQ(step, 1); ++step; });
    sm.newState(StateId::k1,    [&](Event){ EXPECT_EQ(step, 3); ++step; }, [&](Event){ EXPECT_EQ(step, 9); ++step; });
    sm.newState(StateId::kTerm, [&](Event){ EXPECT_EQ(step, 11); ++step; }, [&](Event){ EXPECT_EQ(step, 12); ++step; });

    sm.addRoute(StateId::kInit, EventId::k1, StateId::k1,    nullptr, [&](Event){ EXPECT_EQ(step, 2); ++step; });
    sm.addRoute(StateId::k1,    EventId::k1, StateId::kTerm, nullptr, [&](Event){ EXPECT_EQ(step, 10); ++step; });

    sub_sm.newState(StateId::kInit, [&](Event){ EXPECT_EQ(step, 4); ++step; }, [&](Event){ EXPECT_EQ(step, 5); ++step; });
    sub_sm.newState(StateId::kTerm, [&](Event){ EXPECT_EQ(step, 7); ++step; }, [&](Event){ EXPECT_EQ(step, 8); ++step; });
    sub_sm.addRoute(StateId::kInit, EventId::k1, StateId::kTerm, nullptr, [&](Event){ EXPECT_EQ(step, 6); ++step; });

    sm.setSubStateMachine(StateId::k1, &sub_sm);
#if 0
    sm.setStateChangedCallback(
        [&] (SM::StateID f, SM::StateID t, EventID e) {
            cout << f << "-->" << t << "," << e << endl;
        }
    );
    sub_sm.setStateChangedCallback(
        [&] (SM::StateID f, SM::StateID t, EventID e) {
            cout << "  " <<  f << "-->" << t << "," << e << endl;
        }
    );
#endif
    sm.start();
    sm.run(EventId::k1);
    sm.run(EventId::k1);
    sm.stop();
}

TEST(StateMachine, EventExtra)
{
    enum class StateId { kTerm, kInit };

    SM sm;

    int extra_data = 100;
    int count = 0;
    sm.newState(StateId::kInit,
        [&](Event e){
            ++count;
            EXPECT_EQ(e.id, 0);
            EXPECT_EQ(e.extra, nullptr);
        },
        [&](Event e){
            ++count;
            EXPECT_EQ(e.id, 10);
            EXPECT_EQ(e.extra, &extra_data);
        }
    );
    sm.newState(StateId::kTerm,
        [&](Event e){
            ++count;
            EXPECT_EQ(e.id, 10);
            EXPECT_EQ(e.extra, &extra_data);
        },
        [&](Event e){
            ++count;
            EXPECT_EQ(e.id, 0);
            EXPECT_EQ(e.extra, nullptr);
        }
    );

    sm.addRoute(StateId::kInit, 10, StateId::kTerm,
        [&](Event e){
            ++count;
            EXPECT_EQ(e.id, 10);
            EXPECT_EQ(e.extra, &extra_data);
            return true;
        },
        [&](Event e){
            ++count;
            EXPECT_EQ(e.id, 10);
            EXPECT_EQ(e.extra, &extra_data);
        }
    );

    sm.setStateChangedCallback(
        [&] (SM::StateID f, SM::StateID t, Event e) {
            ++count;
            EXPECT_EQ(e.id, 10);
            EXPECT_EQ(e.extra, &extra_data);
            (void)f;
            (void)t;
        }
    );

    sm.start();
    sm.run(Event(10, &extra_data));
    sm.stop();
    EXPECT_EQ(count, 7);
} 

TEST(StateMachine, InnerEvent) {
    enum class StateId { kTerm, k1, k2 };
    enum class EventId { kAny, k1 };

    SM sm;

    bool is_k1_event_run = false;
    bool is_k2_event_run = false;

    sm.setInitState(StateId::k1);
    sm.newState(StateId::k1, nullptr, nullptr);
    sm.newState(StateId::k2, nullptr, nullptr);
    sm.addEvent(StateId::k1, EventId::k1, [&](Event) { is_k1_event_run = true; return -1; });
    sm.addRoute(StateId::k1, EventId::k1, StateId::k2, nullptr, nullptr);
    sm.addEvent(StateId::k2, EventId::k1, [&](Event) { is_k2_event_run = true; return -1; });

    sm.start();
    sm.run(EventId::k1);
    EXPECT_TRUE(is_k1_event_run);
    EXPECT_FALSE(is_k2_event_run);
    EXPECT_EQ(sm.currentState<StateId>(), StateId::k2);

    is_k1_event_run = false;
    sm.run(EventId::k1);
    EXPECT_FALSE(is_k1_event_run);
    EXPECT_TRUE(is_k2_event_run);
}

TEST(StateMachine, InnerAnyEvent) {
    enum class StateId { kTerm, k1 };
    enum class EventId { kAny, k1, k2 };

    SM sm;

    bool is_k1_event_run = false;
    bool is_any_event_run = false;

    sm.setInitState(StateId::k1);
    sm.newState(StateId::k1, nullptr, nullptr);
    sm.addEvent(StateId::k1, EventId::k1, [&](Event) { is_k1_event_run = true; return -1; });
    sm.addEvent(StateId::k1, EventId::kAny, [&](Event) { is_any_event_run = true; return -1; });

    sm.start();
    sm.run(EventId::k1);
    EXPECT_TRUE(is_k1_event_run);
    EXPECT_FALSE(is_any_event_run);

    is_k1_event_run = false;
    sm.run(EventId::k2);
    EXPECT_FALSE(is_k1_event_run);
    EXPECT_TRUE(is_any_event_run);
}

TEST(StateMachine, SwitchStateInEvent) {
    SM sm;
    sm.newState(STATE_A, nullptr, nullptr);
    sm.newState(STATE_B, nullptr, nullptr);
    sm.setInitState(STATE_A);

    sm.addEvent(STATE_A, EVENT_ANY,
        [](Event e) -> SM::StateID {
            if (e.id == EVENT_1)
                return STATE_B;
            if (e.id == EVENT_2)
                return STATE_TERM;
            return -1;
        }
    );
    sm.addEvent(STATE_B, EVENT_1,
        [](Event e) -> SM::StateID { return STATE_A; (void)e; }
    );
    sm.start();

    EXPECT_EQ(sm.currentState(), STATE_A);
    EXPECT_TRUE(sm.run(EVENT_1));
    EXPECT_EQ(sm.currentState(), STATE_B);
    EXPECT_FALSE(sm.run(EVENT_2));
    EXPECT_TRUE(sm.run(EVENT_1));
    EXPECT_EQ(sm.currentState(), STATE_A);
    EXPECT_TRUE(sm.run(EVENT_2));
    EXPECT_EQ(sm.currentState(), STATE_TERM);
    EXPECT_TRUE(sm.isTerminated());
}

TEST(StateMachine, SetInitState)
{
    enum class StateId { kTerm, k1, kInit };
    enum class EventId { kAny, k1 };

    SM sm;

    sm.setInitState(StateId::kInit);
    sm.newState(StateId::kInit, nullptr, nullptr);
    sm.newState(StateId::k1,    nullptr, nullptr);

    EXPECT_TRUE(sm.start());
    EXPECT_EQ(sm.currentState<StateId>(), StateId::kInit);
}

TEST(StateMachine, SetInitState_Fail)
{
    enum class StateId { kTerm, k1, kInit };
    enum class EventId { kAny, k1 };

    SM sm;

    sm.setInitState(StateId::kInit);
    sm.newState(StateId::k1,    nullptr, nullptr);

    EXPECT_FALSE(sm.start());
}

TEST(StateMachine, LastNextState)
{
    enum class StateId {
        kNone = -1,
        kTerm = 0,
        kInit,
        kA
    };
    enum class EventId {
        kAny,
        k1
    };

    SM sm;

    int count = 0;
    sm.setInitState(StateId::kInit);
    sm.newState(StateId::kInit,
        [&] (Event) {
            ++count;
            EXPECT_EQ(sm.currentState<StateId>(), StateId::kInit);
            EXPECT_EQ(sm.lastState(), -1);
            EXPECT_EQ(sm.nextState(), -1);
        },
        [&] (Event) {
            ++count;
            EXPECT_EQ(sm.currentState<StateId>(), StateId::kInit);
            EXPECT_EQ(sm.lastState(), -1);
            EXPECT_EQ(sm.nextState<StateId>(), StateId::kA);
        }
    );
    sm.newState(StateId::kA,
        [&] (Event) {
            ++count;
            EXPECT_EQ(sm.currentState<StateId>(), StateId::kA);
            EXPECT_EQ(sm.lastState<StateId>(), StateId::kInit);
            EXPECT_EQ(sm.nextState(), -1);
        },
        [&] (Event) {
            ++count;
            EXPECT_EQ(sm.currentState<StateId>(), StateId::kA);
            EXPECT_EQ(sm.lastState<StateId>(), StateId::kInit);
            EXPECT_EQ(sm.nextState<StateId>(), StateId::kTerm);
        }
    );
    sm.newState(StateId::kTerm,
        [&] (Event) {
            ++count;
            EXPECT_EQ(sm.currentState<StateId>(), StateId::kTerm);
            EXPECT_EQ(sm.lastState<StateId>(), StateId::kA);
            EXPECT_EQ(sm.nextState(), -1);
        },
        nullptr
    );

    sm.addRoute(StateId::kInit, EventId::k1, StateId::kA, nullptr,
        [&] (Event) {
            ++count;
            EXPECT_EQ(sm.currentState(), -1);
            EXPECT_EQ(sm.lastState<StateId>(), StateId::kInit);
            EXPECT_EQ(sm.nextState<StateId>(), StateId::kA);
        }
    );
    sm.addRoute(StateId::kA, EventId::k1, StateId::kTerm, nullptr,
        [&] (Event) {
            ++count;
            EXPECT_EQ(sm.currentState(), -1);
            EXPECT_EQ(sm.lastState<StateId>(), StateId::kA);
            EXPECT_EQ(sm.nextState<StateId>(), StateId::kTerm);
        }
    );

    sm.start();

    sm.run(EventId::k1);
    sm.run(EventId::k1);

    EXPECT_EQ(count, 7);
}

}
}
