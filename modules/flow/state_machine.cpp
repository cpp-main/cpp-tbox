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
 * Copyright (c) 2023 Hevake and contributors, all rights reserved.
 *
 * This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
 * Use of this source code is governed by MIT license that can be found
 * in the LICENSE file in the root of the source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */
#include "state_machine.h"

#include <vector>
#include <map>
#include <stdexcept>
#include <algorithm>

#include <tbox/base/log.h>
#include <tbox/base/assert.h>
#include <tbox/base/json.hpp>

namespace tbox {
namespace flow {

using namespace std;

class StateMachine::Impl {
  public:
    ~Impl();

  public:
    bool newState(StateID state_id,
                  const ActionFunc &enter_action,
                  const ActionFunc &exit_action,
                  const std::string &lable);

    bool addRoute(StateID from_state_id,
                  EventID event_id,
                  StateID to_state_id,
                  const GuardFunc &guard,
                  const ActionFunc &action,
                  const std::string &label);

    bool addEvent(StateID state_id,
                  EventID event_id,
                  const EventFunc &action);

    void setInitState(StateID init_state_id) { init_state_id_ = init_state_id; }

    bool setSubStateMachine(StateID state_id, StateMachine *wp_sub_sm);
    void setStateChangedCallback(const StateChangedCallback &cb);

    bool start();
    void stop();

    bool run(Event event);

    StateID currentState() const;
    StateID lastState() const;
    StateID nextState() const;

    bool isTerminated() const;

    void toJson(Json &js) const;

  private:
    struct Route {
        EventID event_id;
        StateID next_state_id;
        GuardFunc  guard;
        ActionFunc action;
        std::string label;
    };

    struct State {
        StateID id;
        ActionFunc enter_action;
        ActionFunc exit_action;
        std::string label;
        StateMachine::Impl *sub_sm; //! 子状态机
        vector<Route> routes;   //! 转换路由
        map<EventID, EventFunc> events;  //! 内部事件
        EventFunc default_event;
    };

    State* findState(StateID state_id) const;

    StateID init_state_id_ = 1;     //! 初始状态

    State *last_state_ = nullptr;   //! 上一个状态指针
    State *curr_state_ = nullptr;   //! 当前状态指针
    State *next_state_ = nullptr;   //! 下一个状态指针
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

bool StateMachine::newState(StateID state_id, const ActionFunc &enter_action, const ActionFunc &exit_action, const std::string &label)
{
    return impl_->newState(state_id, enter_action, exit_action, label);
}

bool StateMachine::addRoute(StateID from_state_id, EventID event_id, StateID to_state_id,
                            const GuardFunc &guard, const ActionFunc &action, const std::string &label)
{
    return impl_->addRoute(from_state_id, event_id, to_state_id, guard, action, label);
}

bool StateMachine::addEvent(StateID state_id, EventID event_id, const EventFunc &action)
{
    return impl_->addEvent(state_id, event_id, action);
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

bool StateMachine::restart()
{
    impl_->stop();
    return impl_->start();
}

bool StateMachine::run(Event event)
{
    return impl_->run(event);
}

StateMachine::StateID StateMachine::currentState() const
{
    return impl_->currentState();
}

StateMachine::StateID StateMachine::lastState() const
{
    return impl_->lastState();
}

StateMachine::StateID StateMachine::nextState() const
{
    return impl_->nextState();
}

bool StateMachine::isTerminated() const
{
    return impl_->isTerminated();
}

void StateMachine::toJson(Json &js) const
{
    return impl_->toJson(js);
}

///////////////////////

StateMachine::Impl::State StateMachine::Impl::_term_state_ = { 0, nullptr, nullptr, "Term", nullptr, { } };

StateMachine::Impl::~Impl()
{
    TBOX_ASSERT(cb_level_ == 0);

    for (auto &item : states_)
        delete item.second;

    states_.clear();
}

bool StateMachine::Impl::newState(StateID state_id,
                                  const ActionFunc &enter_action,
                                  const ActionFunc &exit_action,
                                  const std::string &label)
{
    //! 已存在的状态不能再创建了
    if (states_.find(state_id) != states_.end()) {
        LogWarn("state %u exist", state_id);
        return false;
    }

    auto new_state = new State { state_id, enter_action, exit_action, label, nullptr, { } };
    states_[state_id] = new_state;

    return true;
}

bool StateMachine::Impl::addRoute(StateID from_state_id,
                                  EventID event_id,
                                  StateID to_state_id,
                                  const GuardFunc &guard,
                                  const ActionFunc &action,
                                  const std::string &label)
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

    from_state->routes.emplace_back(Route{ event_id, to_state_id, guard, action, label });
    return true;
}

bool StateMachine::Impl::addEvent(StateID state_id,
                                  EventID event_id,
                                  const EventFunc &action)
{
    if (action == nullptr) {
        LogWarn("action is nullptr", state_id);
        return false;
    }

    //! 要求 state_id 必须是已存在的状态
    auto state = findState(state_id);
    if (state == nullptr) {
        LogWarn("state %d not exist", state_id);
        return false;
    }

    if (event_id != 0)
        state->events[event_id] = action;
    else
        state->default_event = action;

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

    curr_state_ = init_state;

    ++cb_level_;
    if (init_state->enter_action)
        init_state->enter_action(Event());
    --cb_level_;

    //! 如果有子状态机，在启动子状态机
    if (curr_state_->sub_sm != nullptr)
        curr_state_->sub_sm->start();

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
        curr_state_->exit_action(Event());
    --cb_level_;

    curr_state_ = nullptr;
}

bool StateMachine::Impl::run(Event event)
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
        bool ret = curr_state_->sub_sm->run(event);
        if (!curr_state_->sub_sm->isTerminated())
            return ret;
        curr_state_->sub_sm->stop();
    }

    StateID next_state_id = -1;
    ActionFunc route_action;

    //! 检查事件，并执行
    ++cb_level_;
    auto event_iter = curr_state_->events.find(event.id);
    if (event_iter != curr_state_->events.end())
        next_state_id = event_iter->second(event);
    else if (curr_state_->default_event)
        next_state_id = curr_state_->default_event(event);
    --cb_level_;

    if (next_state_id < 0) {
        //! 找出可行的路径
        ++cb_level_;
        auto route_iter = std::find_if(curr_state_->routes.begin(), curr_state_->routes.end(),
            [event] (const Route &item) -> bool {
                if (item.event_id != 0 && item.event_id != event.id)
                    return false;
                if (item.guard != nullptr && !item.guard(event))
                    return false;
                return true;
            }
        );
        --cb_level_;

        //! 如果没有跳转则直接退出
        if (route_iter == curr_state_->routes.end())
            return false;

        //! 以下是有跳转的情况
        next_state_id = route_iter->next_state_id;
        route_action = route_iter->action;
    }

    next_state_ = findState(next_state_id);
    if (next_state_ == nullptr) {
        if (next_state_id == 0) {
            next_state_ = &_term_state_;
        } else {
            LogErr("Should not happen");
            return false;
        }
    }

    ++cb_level_;
    if (curr_state_->exit_action)
        curr_state_->exit_action(event);

    last_state_ = curr_state_;
    curr_state_ = nullptr;

    if (route_action)
        route_action(event);

    curr_state_ = next_state_;
    next_state_ = nullptr;

    if (curr_state_->enter_action)
        curr_state_->enter_action(event);

    if (state_changed_cb_)
        state_changed_cb_(last_state_->id, curr_state_->id, event);

    //! 如果新的状态有子状态机，则启动子状态机并将事件交给子状态机处理
    if (curr_state_->sub_sm != nullptr) {
        curr_state_->sub_sm->start();
        curr_state_->sub_sm->run(event);
    }
    --cb_level_;

    return true;
}

StateMachine::StateID StateMachine::Impl::currentState() const
{
    if (curr_state_ != nullptr)
        return curr_state_->id;
    return -1;
}

StateMachine::StateID StateMachine::Impl::lastState() const
{
    if (last_state_ != nullptr)
        return last_state_->id;
    return -1;
}

StateMachine::StateID StateMachine::Impl::nextState() const
{
    if (next_state_ != nullptr)
        return next_state_->id;
    return -1;
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

void StateMachine::Impl::toJson(Json &js) const
{
    js["init_state"] = init_state_id_;
    js["term_state"] = 0;
    if (curr_state_ != nullptr)
        js["curr_state"] = curr_state_->id;

    auto &js_state_array = js["states"];
    for (auto &item : states_) {
        auto state = item.second;
        Json js_state;
        js_state["id"] = state->id;
        js_state["label"] = state->label;

        if (state->sub_sm != nullptr)
            state->sub_sm->toJson(js_state["sub_sm"]);

        auto &js_route_array = js_state["routes"];
        for (auto &route : state->routes) {
            Json js_route;
            js_route["event_id"] = route.event_id;
            js_route["next_state_id"] = route.next_state_id;
            js_route["label"] = route.label;
            js_route_array.push_back(std::move(js_route));
        }

        auto &js_event_array = js_state["events"];
        for (auto &item : state->events)
            js_event_array.push_back(item.first);

        js_state_array.push_back(std::move(js_state));
    }
}

}
}
