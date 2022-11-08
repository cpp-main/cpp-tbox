#include <gtest/gtest.h>

#include "state_machine.h"

using namespace std;

namespace tbox {
namespace util {

using SM = StateMachine;

enum State {
    STATE_A = 1,
    STATE_B,
    STATE_C,
    STATE_D,
};

enum Event {
    EVENT_1 = 1,
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
    sm.addRoute(STATE_A, EVENT_1, STATE_B, [&](SM::Event){ return !condition; }, nullptr);
    sm.addRoute(STATE_A, EVENT_1, STATE_C, [&](SM::Event){ return condition; },  nullptr);
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

    sm.newState(STATE_A, [&](SM::Event){ ++a_enter_counter; }, [&](SM::Event){ ++a_exit_counter; });
    sm.newState(STATE_B, [&](SM::Event){ ++b_enter_counter; }, [&](SM::Event){ ++b_exit_counter; });

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

    sm.addRoute(STATE_A, EVENT_1, STATE_B, nullptr, [&](SM::Event){ ++count_1; });
    sm.addRoute(STATE_B, EVENT_2, STATE_A, nullptr, [&](SM::Event){ ++count_2; });
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

    sm.newState(STATE_A, [&](SM::Event){ ++a_enter_counter; }, [&](SM::Event){ ++a_exit_counter; });
    sm.newState(STATE_B, [&](SM::Event){ ++b_enter_counter; }, [&](SM::Event){ ++b_exit_counter; });

    sm.addRoute(STATE_A, EVENT_1, STATE_B, nullptr, nullptr);
    sm.setInitState(STATE_A);
    sm.start();
    EXPECT_EQ(a_enter_counter, 1);
    EXPECT_TRUE(sm.run(EVENT_1));
    EXPECT_EQ(a_exit_counter, 1);
    EXPECT_EQ(b_enter_counter, 1);
    sm.stop();
    EXPECT_EQ(b_exit_counter, 1);
    sm.start();
    EXPECT_EQ(a_enter_counter, 2);
}

//! 主要测试用 enum class 定义的状态与事件是否能编译过
TEST(StateMachine, EnumClass)
{
    SM sm;
    enum class State { k1 = 1, k2 };
    enum class Event { k1 = 1, k2 };

    int enter_counter = 0;
    int exit_counter = 0;

    sm.newState(State::k1, [&](SM::Event){ ++enter_counter; }, [&](SM::Event){ ++exit_counter; });
    sm.newState(State::k2, nullptr, nullptr);

    sm.addRoute(State::k1, Event::k1, State::k2, nullptr, nullptr);
    sm.addRoute(State::k2, Event::k2, State::k1, nullptr, nullptr);
    sm.setInitState(State::k1);
    sm.start();
    sm.run(Event::k1);
    sm.run(Event::k2);
    sm.stop();

    EXPECT_EQ(enter_counter, 2);
    EXPECT_EQ(exit_counter, 2);
}

TEST(StateMachine, SubSM)
{
    enum class State { kTerm, kInit, k1, k2 };
    enum class Event { kTerm, k1, k2 };

    SM sm, sub_sm;

    sm.newState(State::kInit, nullptr, nullptr);
    sm.newState(State::k1,    nullptr, nullptr);
    sm.newState(State::k2,    nullptr, nullptr);

    sm.addRoute(State::kInit, Event::k1, State::k1,    nullptr, nullptr);
    sm.addRoute(State::kInit, Event::k2, State::k2,    nullptr, nullptr);
    sm.addRoute(State::k1,    Event::k1, State::kTerm, nullptr, nullptr);
    sm.addRoute(State::k1,    Event::k2, State::k2,    nullptr, nullptr);
    sm.addRoute(State::k2,    Event::k1, State::k1,    nullptr, nullptr);
    sm.addRoute(State::k2,    Event::k2, State::k1,    nullptr, nullptr);

    sub_sm.newState(State::kInit, nullptr, nullptr);
    sub_sm.newState(State::k1,    nullptr, nullptr);
    sub_sm.newState(State::k2,    nullptr, nullptr);

    sub_sm.addRoute(State::kInit, Event::k1, State::k1,    nullptr, nullptr);
    sub_sm.addRoute(State::kInit, Event::k2, State::k2,    nullptr, nullptr);
    sub_sm.addRoute(State::k1,    Event::k2, State::k2,    nullptr, nullptr);
    sub_sm.addRoute(State::k1,    Event::k1, State::kTerm, nullptr, nullptr);
    sub_sm.addRoute(State::k2,    Event::k1, State::k1,    nullptr, nullptr);
    sub_sm.addRoute(State::k2,    Event::k2, State::kTerm, nullptr, nullptr);

    sm.setSubStateMachine(State::k1, &sub_sm);

    {   //! 直通
        sm.start();

        EXPECT_EQ(sm.currentState<State>(), State::kInit);
        EXPECT_EQ(sub_sm.currentState<State>(), State::kTerm);

        EXPECT_TRUE(sm.run(Event::k1));
        EXPECT_EQ(sm.currentState<State>(), State::k1);
        EXPECT_EQ(sub_sm.currentState<State>(), State::k1);

        EXPECT_TRUE(sm.run(Event::k1));
        EXPECT_EQ(sm.currentState<State>(), State::kTerm);
        EXPECT_EQ(sub_sm.currentState<State>(), State::kTerm);

        EXPECT_TRUE(sm.isTerminated());

        sm.stop();
    }

    {   //! 在子状态机里转了一下
        sm.start();

        EXPECT_EQ(sm.currentState<State>(), State::kInit);
        EXPECT_EQ(sub_sm.currentState<State>(), State::kTerm);

        EXPECT_TRUE(sm.run(Event::k1));
        EXPECT_EQ(sm.currentState<State>(), State::k1);
        EXPECT_EQ(sub_sm.currentState<State>(), State::k1);

        EXPECT_TRUE(sm.run(Event::k2));
        EXPECT_EQ(sm.currentState<State>(), State::k1);
        EXPECT_EQ(sub_sm.currentState<State>(), State::k2);

        EXPECT_TRUE(sm.run(Event::k1));
        EXPECT_EQ(sm.currentState<State>(), State::k1);
        EXPECT_EQ(sub_sm.currentState<State>(), State::k1);

        EXPECT_TRUE(sm.run(Event::k1));
        EXPECT_EQ(sm.currentState<State>(), State::kTerm);
        EXPECT_EQ(sub_sm.currentState<State>(), State::kTerm);
        EXPECT_TRUE(sm.isTerminated());

        sm.stop();
    }

    {   //! 从S2进的S1的子状态机，再出来回到S2，再进去
        sm.start();

        EXPECT_EQ(sm.currentState<State>(), State::kInit);
        EXPECT_EQ(sub_sm.currentState<State>(), State::kTerm);

        EXPECT_TRUE(sm.run(Event::k2));
        EXPECT_EQ(sm.currentState<State>(), State::k2);
        EXPECT_EQ(sub_sm.currentState<State>(), State::kTerm);

        EXPECT_TRUE(sm.run(Event::k2));
        EXPECT_EQ(sm.currentState<State>(), State::k1);
        EXPECT_EQ(sub_sm.currentState<State>(), State::k2);

        EXPECT_TRUE(sm.run(Event::k2));
        EXPECT_EQ(sm.currentState<State>(), State::k2);
        EXPECT_EQ(sub_sm.currentState<State>(), State::kTerm);

        EXPECT_TRUE(sm.run(Event::k2));
        EXPECT_EQ(sm.currentState<State>(), State::k1);
        EXPECT_EQ(sub_sm.currentState<State>(), State::k2);

        EXPECT_TRUE(sm.run(Event::k1));
        EXPECT_EQ(sm.currentState<State>(), State::k1);
        EXPECT_EQ(sub_sm.currentState<State>(), State::k1);

        EXPECT_TRUE(sm.run(Event::k1));
        EXPECT_EQ(sm.currentState<State>(), State::kTerm);
        EXPECT_EQ(sub_sm.currentState<State>(), State::kTerm);

        sm.stop();
    }
}

TEST(StateMachine, StateChangedCallback)
{
    enum class State { kTerm, kInit, k1 };
    enum class Event { kNone, k1 };

    SM sm;

    sm.newState(State::kInit, nullptr, nullptr);
    sm.newState(State::k1,    nullptr, nullptr);

    sm.addRoute(State::kInit, Event::k1, State::k1,    nullptr, nullptr);
    sm.addRoute(State::k1,    Event::k1, State::kTerm, nullptr, nullptr);

    State from, to;
    Event event;

    sm.setStateChangedCallback(
        [&] (SM::StateID f, SM::StateID t, SM::Event e) {
            from = static_cast<State>(f);
            to = static_cast<State>(t);
            event = static_cast<Event>(e.id);
        }
    );
    sm.start();
    sm.run(Event::k1);
    EXPECT_EQ(from, State::kInit);
    EXPECT_EQ(to, State::k1);
    EXPECT_EQ(event, Event::k1);

    sm.run(Event::k1);
    EXPECT_EQ(from, State::k1);
    EXPECT_EQ(to, State::kTerm);
    EXPECT_EQ(event, Event::k1);
}

//! 测试起始状态有子状态的情况
TEST(StateMachine, InitStateHasSubMachine)
{
    enum class State { kTerm, kInit };
    enum class Event { kNone, k1 };

    SM sm;
    SM sub_sm;

    int step = 0;
    sm.newState(State::kInit, [&](SM::Event){ EXPECT_EQ(step, 0); ++step; }, [&](SM::Event){ EXPECT_EQ(step, 6); ++step; });
    sm.newState(State::kTerm, [&](SM::Event){ EXPECT_EQ(step, 8); ++step; }, [&](SM::Event){ EXPECT_EQ(step, 9); ++step; });
    sm.addRoute(State::kInit, Event::k1, State::kTerm, nullptr, [&](SM::Event){ EXPECT_EQ(step, 7); ++step; });

    sub_sm.newState(State::kInit, [&](SM::Event){ EXPECT_EQ(step, 1); ++step; }, [&](SM::Event){ EXPECT_EQ(step, 2); ++step; });
    sub_sm.newState(State::kTerm, [&](SM::Event){ EXPECT_EQ(step, 4); ++step; }, [&](SM::Event){ EXPECT_EQ(step, 5); ++step; });
    sub_sm.addRoute(State::kInit, Event::k1, State::kTerm, nullptr, [&](SM::Event){ EXPECT_EQ(step, 3); ++step; });

    sm.setSubStateMachine(State::kInit, &sub_sm);
#if 0
    sm.setStateChangedCallback(
        [&] (SM::StateID f, SM::StateID t, SM::EventID e) {
            cout << f << "-->" << t << "," << e << endl;
        }
    );
    sub_sm.setStateChangedCallback(
        [&] (SM::StateID f, SM::StateID t, SM::EventID e) {
            cout << "  " <<  f << "-->" << t << "," << e << endl;
        }
    );
#endif
    sm.start();
    sm.run(Event::k1);
    sm.stop();
}

//! 测试各种Action之间的执行顺序
TEST(StateMachine, SubSMActionOrder)
{
    enum class State { kTerm, kInit, k1};
    enum class Event { kNone, k1 };

    SM sm;
    SM sub_sm;

    int step = 0;
    sm.newState(State::kInit, [&](SM::Event){ EXPECT_EQ(step, 0); ++step; }, [&](SM::Event){ EXPECT_EQ(step, 1); ++step; });
    sm.newState(State::k1,    [&](SM::Event){ EXPECT_EQ(step, 3); ++step; }, [&](SM::Event){ EXPECT_EQ(step, 9); ++step; });
    sm.newState(State::kTerm, [&](SM::Event){ EXPECT_EQ(step, 11); ++step; }, [&](SM::Event){ EXPECT_EQ(step, 12); ++step; });

    sm.addRoute(State::kInit, Event::k1, State::k1,    nullptr, [&](SM::Event){ EXPECT_EQ(step, 2); ++step; });
    sm.addRoute(State::k1,    Event::k1, State::kTerm, nullptr, [&](SM::Event){ EXPECT_EQ(step, 10); ++step; });

    sub_sm.newState(State::kInit, [&](SM::Event){ EXPECT_EQ(step, 4); ++step; }, [&](SM::Event){ EXPECT_EQ(step, 5); ++step; });
    sub_sm.newState(State::kTerm, [&](SM::Event){ EXPECT_EQ(step, 7); ++step; }, [&](SM::Event){ EXPECT_EQ(step, 8); ++step; });
    sub_sm.addRoute(State::kInit, Event::k1, State::kTerm, nullptr, [&](SM::Event){ EXPECT_EQ(step, 6); ++step; });

    sm.setSubStateMachine(State::k1, &sub_sm);
#if 0
    sm.setStateChangedCallback(
        [&] (SM::StateID f, SM::StateID t, SM::EventID e) {
            cout << f << "-->" << t << "," << e << endl;
        }
    );
    sub_sm.setStateChangedCallback(
        [&] (SM::StateID f, SM::StateID t, SM::EventID e) {
            cout << "  " <<  f << "-->" << t << "," << e << endl;
        }
    );
#endif
    sm.start();
    sm.run(Event::k1);
    sm.run(Event::k1);
    sm.stop();
}

TEST(StateMachine, EventExtra)
{
    enum class State { kTerm, kInit };

    SM sm;

    int extra_data = 100;
    int count = 0;
    sm.newState(State::kInit,
        [&](SM::Event e){
            ++count;
            EXPECT_EQ(e.id, 0);
            EXPECT_EQ(e.extra, nullptr);
        },
        [&](SM::Event e){
            ++count;
            EXPECT_EQ(e.id, 10);
            EXPECT_EQ(e.extra, &extra_data);
        }
    );
    sm.newState(State::kTerm,
        [&](SM::Event e){
            ++count;
            EXPECT_EQ(e.id, 10);
            EXPECT_EQ(e.extra, &extra_data);
        },
        [&](SM::Event e){
            ++count;
            EXPECT_EQ(e.id, 0);
            EXPECT_EQ(e.extra, nullptr);
        }
    );

    sm.addRoute(State::kInit, 10, State::kTerm,
        [&](SM::Event e){
            ++count;
            EXPECT_EQ(e.id, 10);
            EXPECT_EQ(e.extra, &extra_data);
            return true;
        },
        [&](SM::Event e){
            ++count;
            EXPECT_EQ(e.id, 10);
            EXPECT_EQ(e.extra, &extra_data);
        }
    );

    sm.setStateChangedCallback(
        [&] (SM::StateID f, SM::StateID t, SM::Event e) {
            ++count;
            EXPECT_EQ(e.id, 10);
            EXPECT_EQ(e.extra, &extra_data);
        }
    );

    sm.start();
    sm.run(SM::Event(10, &extra_data));
    sm.stop();
    EXPECT_EQ(count, 7);
} 

TEST(StateMachine, InnerEvent) {
    enum class State { kTerm, k1, k2 };
    enum class Event { kNone, k1 };

    SM sm;

    bool is_k1_event_run = false;
    bool is_k2_event_run = false;

    sm.setInitState(State::k1);
    sm.newState(State::k1, nullptr, nullptr);
    sm.newState(State::k2, nullptr, nullptr);
    sm.addEvent(State::k1, Event::k1, [&](SM::Event) { is_k1_event_run = true; });
    sm.addRoute(State::k1, Event::k1, State::k2, nullptr, nullptr);
    sm.addEvent(State::k2, Event::k1, [&](SM::Event) { is_k2_event_run = true; });

    sm.start();
    sm.run(Event::k1);
    EXPECT_TRUE(is_k1_event_run);
    EXPECT_FALSE(is_k2_event_run);
    EXPECT_EQ(sm.currentState<State>(), State::k2);

    is_k1_event_run = false;
    sm.run(Event::k1);
    EXPECT_FALSE(is_k1_event_run);
    EXPECT_TRUE(is_k2_event_run);
}

TEST(StateMachine, SetInitState)
{
    enum class State { kTerm, k1, kInit };
    enum class Event { kNone, k1 };

    SM sm;

    sm.setInitState(State::kInit);
    sm.newState(State::kInit, nullptr, nullptr);
    sm.newState(State::k1,    nullptr, nullptr);

    EXPECT_TRUE(sm.start());
    EXPECT_EQ(sm.currentState<State>(), State::kInit);
}

TEST(StateMachine, SetInitState_Fail)
{
    enum class State { kTerm, k1, kInit };
    enum class Event { kNone, k1 };

    SM sm;

    sm.setInitState(State::kInit);
    sm.newState(State::k1,    nullptr, nullptr);

    EXPECT_FALSE(sm.start());
}

}
}
