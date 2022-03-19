#include "state_machine.h"
#include <tbox/base/log.h>

namespace tbox {
namespace sm {

StateMachine::StateMachine()
{ }

StateMachine::~StateMachine()
{ }

bool StateMachine::newState(StateID state, const ActionFunc &enter_action, const ActionFunc &exit_action)
{
    LogUndo();
    return false;
}

void StateMachine::setInitState(StateID state)
{
    LogUndo();
}

bool StateMachine::addRoute(StateID from, EventID event, StateID to, const GuardFunc &guard, const ActionFunc &action)
{
    LogUndo();
    return false;
}

bool StateMachine::run(EventID event)
{
    LogUndo();
    return false;
}

StateMachine::StateID StateMachine::currentState() const
{
    LogUndo();
    return 0;
}

}
}
