#include "state_machine.h"
#include <tbox/base/log.h>

#include <cassert>
#include <vector>
#include <map>
#include <algorithm>

namespace tbox {
namespace util {

using namespace std;

class StateMachine::Impl {
  public:
    ~Impl();

  public:
    bool newState(StateID state_id, const ActionFunc &enter_action, const ActionFunc &exit_action);
    bool addRoute(StateID from_state_id, EventID event_id, StateID to_state_id, const GuardFunc &guard, const ActionFunc &action);
    void setInitState(StateID init_state_id) { init_state_id_ = init_state_id; }

    bool setSubStateMachine(StateID state_id, StateMachine *wp_sub_sm);
    void setStateChangedCallback(const StateChangedCallback &cb);

    bool start();
    void stop();

    bool run(EventID event_id);
    StateID currentState() const;
    bool isTerminated() const;

  private:
    struct Route {
        EventID event_id;
        StateID next_state_id;
        GuardFunc  guard;
        ActionFunc action;
    };

    struct State {
        StateID id;
        ActionFunc enter_action;
        ActionFunc exit_action;
        StateMachine::Impl *sub_sm; //! 子状态机
        vector<Route> routes;
    };

    State* findState(StateID state_id) const;

    StateID init_state_id_ = 1;     //! 初始状态

    State *curr_state_ = nullptr;   //! 当前状态指针
    map<StateID, State*> states_;   //! 状态对象表

    StateChangedCallback state_changed_cb_;

    static State _term_state_;  //! 默认终止状态对象

    int cb_level_ = 0;
};

StateMachine::StateMachine() : impl_(new Impl)
{ }

StateMachine::~StateMachine()
{
    delete impl_;
}

bool StateMachine::newState(StateID state_id, const ActionFunc &enter_action, const ActionFunc &exit_action)
{
    return impl_->newState(state_id, enter_action, exit_action);
}

bool StateMachine::addRoute(StateID from_state_id, EventID event_id, StateID to_state_id,
                            const GuardFunc &guard, const ActionFunc &action)
{
    return impl_->addRoute(from_state_id, event_id, to_state_id, guard, action);
}

void StateMachine::setInitState(StateID init_state_id)
{
    impl_->setInitState(init_state_id);
}

bool StateMachine::setSubStateMachine(StateID state_id, StateMachine *wp_sub_sm)
{
    return impl_->setSubStateMachine(state_id, wp_sub_sm);
}

void StateMachine::setStateChangedCallback(const StateChangedCallback &cb)
{
    impl_->setStateChangedCallback(cb);
}

bool StateMachine::start()
{
    return impl_->start();
}

void StateMachine::stop()
{
    impl_->stop();
}

bool StateMachine::run(EventID event_id)
{
    return impl_->run(event_id);
}

StateMachine::StateID StateMachine::currentState() const
{
    return impl_->currentState();
}

bool StateMachine::isTerminated() const
{
    return impl_->isTerminated();
}

///////////////////////

StateMachine::Impl::State StateMachine::Impl::_term_state_ = { 0 };

StateMachine::Impl::~Impl()
{
    assert(cb_level_ == 0);

    for (auto &item : states_)
        delete item.second;

    states_.clear();
}

bool StateMachine::Impl::newState(StateID state_id, const ActionFunc &enter_action, const ActionFunc &exit_action)
{
    //! 已存在的状态不能再创建了
    if (states_.find(state_id) != states_.end()) {
        LogWarn("state %u exist", state_id);
        return false;
    }

    auto new_state = new State { state_id, enter_action, exit_action, nullptr };
    states_[state_id] = new_state;

    return true;
}

bool StateMachine::Impl::addRoute(StateID from_state_id, EventID event_id, StateID to_state_id,
                                  const GuardFunc &guard, const ActionFunc &action)
{
    //! 要求 from_state_id 必须是已存在的状态
    auto from_state = findState(from_state_id);
    if (from_state == nullptr) {
        LogWarn("from_state %d not exist", from_state_id);
        return false;
    }

    //! 如果 to_state_id 为是终止状态，那么这个状态必须是已存在的
    if (to_state_id != 0 && findState(to_state_id) == nullptr) {
        LogWarn("to_state %d not exist", to_state_id);
        return false;
    }

    from_state->routes.emplace_back(Route{ event_id, to_state_id, guard, action });
    return true;
}

bool StateMachine::Impl::setSubStateMachine(StateID state_id, StateMachine *wp_sub_sm)
{
    auto state = findState(state_id);
    if (state == nullptr) {
        LogWarn("state:%u not exist", state_id);
        return false;
    }

    state->sub_sm = wp_sub_sm->impl_;
    return true;
}

void StateMachine::Impl::setStateChangedCallback(const StateChangedCallback &cb)
{
    state_changed_cb_ = cb;
}

bool StateMachine::Impl::start()
{
    if (curr_state_ != nullptr) {
        LogWarn("it's already started");
        return false;
    }

    if (cb_level_ != 0) {
        LogWarn("recursion invoke");
        return false;
    }

    auto init_state = findState(init_state_id_);
    if (init_state == nullptr) {
        LogWarn("init state %u not exist", init_state_id_);
        return false;
    }

    ++cb_level_;
    if (init_state->enter_action)
        init_state->enter_action();
    --cb_level_;

    curr_state_ = init_state;
    return true;
}

void StateMachine::Impl::stop()
{
    if (curr_state_ == nullptr)
        return;

    if (cb_level_ != 0) {
        LogWarn("recursion invoke");
        return;
    }

    ++cb_level_;
    if (curr_state_->exit_action)
        curr_state_->exit_action();
    --cb_level_;

    curr_state_ = nullptr;
}

bool StateMachine::Impl::run(EventID event_id)
{
    if (curr_state_ == nullptr) {
        LogWarn("need start first");
        return false;
    }

    if (cb_level_ != 0) {
        LogWarn("recursion invoke");
        return false;
    }

    //! 如果有子状态机，则给子状态机处理
    if (curr_state_->sub_sm != nullptr) {
        bool ret = curr_state_->sub_sm->run(event_id);
        if (!curr_state_->sub_sm->isTerminated())
            return ret;
        curr_state_->sub_sm->stop();
    }

    //! 找出可行的路径
    auto iter = std::find_if(curr_state_->routes.begin(), curr_state_->routes.end(),
        [event_id] (const Route &item) -> bool {
            if (item.event_id != 0 && item.event_id != event_id)
                return false;
            if (item.guard != nullptr && !item.guard())
                return false;
            return true;
        }
    );

    //! 如果没有跳转则直接退出
    if (iter == curr_state_->routes.end())
        return false;

    //! 以下是有跳转的情况
    const Route &route = *iter;
    auto last_state = curr_state_;

    ++cb_level_;

    if (curr_state_->exit_action)
        curr_state_->exit_action();

    if (route.action)
        route.action();

    State *next_state = findState(route.next_state_id);
    if (next_state != nullptr) {
        if (next_state->enter_action)
            next_state->enter_action();

        curr_state_ = next_state;

        //! 如果有子状态机，则给子状态机处理
        if (curr_state_->sub_sm != nullptr) {
            curr_state_->sub_sm->start();
            curr_state_->sub_sm->run(event_id);
        }
    } else if (route.next_state_id == 0) {
        curr_state_ = &_term_state_;
    } else {
        LogErr("Should not happen");
        return false;
    }

    if (state_changed_cb_)
        state_changed_cb_(last_state->id, curr_state_->id, event_id);

    --cb_level_;
    return true;
}

StateMachine::StateID StateMachine::Impl::currentState() const
{
    if (curr_state_ == nullptr) {
        LogWarn("need start first");
        return 0;
    }

    return curr_state_->id;
}

bool StateMachine::Impl::isTerminated() const
{
    if (curr_state_ == nullptr) {
        LogWarn("need start first");
        return false;
    }

    return curr_state_->id == 0;
}

StateMachine::Impl::State* StateMachine::Impl::findState(StateID state_id) const
{
    try {
        return states_.at(state_id);
    } catch (const out_of_range &e) {
        return nullptr;
    }
}

}
}
