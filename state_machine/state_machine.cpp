#include "state_machine.h"
#include <tbox/base/log.h>

namespace tbox {
namespace sm {

StateMachine::StateMachine()
{ }

StateMachine::~StateMachine()
{ }

StateMachine::StateToken StateMachine::newState(StateID state_id, const ActionFunc &enter_action, const ActionFunc &exit_action)
{
    LogUndo();
    return StateToken();
}

void StateMachine::setInitState(StateToken init_state)
{
    LogUndo();
}

void StateMachine::setTermState(StateToken init_state)
{
    LogUndo();
}

bool StateMachine::addRoute(StateToken from, EventID event, StateToken to, const GuardFunc &guard, const ActionFunc &action)
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
