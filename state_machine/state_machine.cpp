#include "state_machine.h"
#include <tbox/base/log.h>

#include <cassert>
#include <vector>
#include <map>
#include <algorithm>

namespace tbox {
namespace sm {

using namespace std;

class StateMachine::Impl {
  public:
    ~Impl();

  public:
    bool newState(StateID state_id, const ActionFunc &enter_action, const ActionFunc &exit_action);
    bool addRoute(StateID from_state_id, EventID event_id, StateID to_state_id, const GuardFunc &guard, const ActionFunc &action);
    bool start(StateID init_state_id);
    bool run(EventID event_id);
    StateID currentState() const;

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
        vector<Route> routes;
    };

    State* findState(StateID state_id) const;

    State *curr_state_ = nullptr;
    map<StateID, State*> states_;

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

bool StateMachine::start(StateID init_state_id)
{
    return impl_->start(init_state_id);
}

bool StateMachine::run(EventID event_id)
{
    return impl_->run(event_id);
}

StateMachine::StateID StateMachine::currentState() const
{
    return impl_->currentState();
}


StateMachine::Impl::~Impl()
{
    assert(cb_level_ == 0);

    for (auto &item : states_)
        delete item.second;

    states_.clear();
}

bool StateMachine::Impl::newState(StateID state_id, const ActionFunc &enter_action, const ActionFunc &exit_action)
{
    if (states_.find(state_id) != states_.end()) {
        LogWarn("state %u exist", state_id);
        return false;
    }

    auto new_state = new State { state_id, enter_action, exit_action };
    states_[state_id] = new_state;

    if (curr_state_ == nullptr)
        curr_state_ = new_state;

    return true;
}

bool StateMachine::Impl::addRoute(StateID from_state_id, EventID event_id, StateID to_state_id,
                                  const GuardFunc &guard, const ActionFunc &action)
{
    auto from_state = findState(from_state_id);
    auto to_state = findState(to_state_id);
    if (from_state == nullptr || to_state == nullptr) {
        LogWarn("either from or to state not exist");
        return false;
    }

    from_state->routes.emplace_back(Route{ event_id, to_state_id, guard, action });
    return true;
}

bool StateMachine::Impl::start(StateID init_state_id)
{
    if (curr_state_ != nullptr) {
        LogWarn("it's already started");
        return false;
    }

    auto init_state = findState(init_state_id);
    if (init_state == nullptr) {
        LogWarn("state %u not found", init_state_id);
        return false;
    }

    ++cb_level_;
    if (init_state->enter_action)
        init_state->enter_action();
    --cb_level_;

    curr_state_ = init_state;
    return true;
}

bool StateMachine::Impl::run(EventID event_id)
{
    if (curr_state_ == nullptr) {
        LogWarn("need start first");
        return false;
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

    if (iter == curr_state_->routes.end())
        return false;

    const Route &route = *iter;
    State *next_state = findState(route.next_state_id);
    assert(next_state != nullptr);

    ++cb_level_;
    if (curr_state_->exit_action)
        curr_state_->exit_action();

    if (route.action)
        route.action();

    if (next_state->enter_action)
        next_state->enter_action();
    --cb_level_;

    curr_state_ = next_state;
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
