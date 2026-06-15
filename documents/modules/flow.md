# Flow Control Module (flow)

## What is it?

The flow module provides two types of flow control tools: multi-level finite state machine (StateMachine) and behavior tree (Action series). StateMachine is used for state-driven business logic, while Action is used for composite complex flows.

## Why do you need it?

In event-driven programming, complex business flows need to manage a large number of states and conditional judgments. StateMachine provides clear state definitions and transition rules, while Action provides composite flow control (sequential, parallel, conditional selection, etc.), separating complex control logic from business code.

![state-machine](../images/0010-state-machine-graph.png)

![action-tree](../images/0010-action-tree-graph.jpg)

## Header Files

```cpp
#include <tbox/flow/state_machine.h>      //! State machine
#include <tbox/flow/action.h>              //! Action base class
#include <tbox/flow/action_executor.h>     //! Action executor
#include <tbox/flow/event.h>               //! Event definitions
#include <tbox/flow/event_publisher.h>      //! Event publisher
#include <tbox/flow/event_subscriber.h>     //! Event subscriber
#include <tbox/flow/to_graphviz.h>         //! Export Graphviz diagram
```

## Core Classes and Interfaces

### StateMachine — Multi-level Finite State Machine

| Method | Description |
|--------|-------------|
| `newState(state_id, enter_action, exit_action, label)` | Create a state |
| `addRoute(from, event, to, guard, action, label)` | Add a state transition route |
| `addEvent(state_id, event_id, action)` | Add an in-state event handler |
| `setInitState(state_id)` | Set the initial state |
| `setSubStateMachine(state_id, sub_sm)` | Set a sub state machine (hierarchical nesting) |
| `setStateChangedCallback(cb)` | Set state change callback |
| `start()` | Start the state machine |
| `stop()` | Stop the state machine |
| `run(event)` | Run the state machine (pass in an event) |
| `currentState()` | Get the current state |
| `lastState()` | Get the previous state |
| `nextState()` | Get the next state (valid during transition) |
| `isRunning()` | Whether it is running |
| `isTerminated()` | Whether it has terminated |

#### State Machine Key Concepts

- **StateID**: State number, 0 is the terminated state, -1 is the invalid state
- **EventID**: Event number, 0 means any event
- **Route**: Defines a conditional route that transitions from one state to another upon receiving a specific event
- **GuardFunc**: Condition judgment function, returns true to indicate the condition is satisfied and the transition can proceed
- **EventFunc**: Event handler function, returns >=0 to indicate a transition to the specified state is needed
- **Sub State Machine**: StateMachine supports nesting; inner state machines can independently manage sub-states

### Action — Behavior Tree Action Base Class

Action is the base node of the behavior tree, providing unified lifecycle management:

| Method | Description |
|--------|-------------|
| `start()` | Start execution |
| `pause()` | Pause |
| `resume()` | Resume |
| `stop()` | Stop |
| `reset()` | Reset to initial state |
| `isReady()` | Whether it is ready (needs subclass implementation) |
| `setFinishCallback(cb)` | Set finish callback |
| `setBlockCallback(cb)` | Set block callback |
| `setTimeout(ms)` | Set timeout duration |
| `finish(is_succ, why, trace)` | Manually finish |
| `block(why, trace)` | Manually pause |

Action states: kIdle (idle) → kRunning (running) → kFinished (finished)/kStoped (stopped)/kPause (paused)

Action results: kUnsure (unknown) → kSuccess (success)/kFail (failure)

#### Action Subclass Types

Action has various composite and functional subclasses (see unit test cases in `modules/flow/` for complete examples):

- **Sequential composition**: SequentialAction (execute multiple sub-actions in order)
- **Parallel composition**: ParallelAction (execute multiple sub-actions simultaneously)
- **Conditional selection**: IfElseAction (conditional branch), SwitchAction (multi-way selection)
- **Loop control**: LoopAction (loop execution), WhileAction (conditional loop)
- **Decorators**: RetryAction (retry on failure), TimeoutAction (timeout control), DelayAction (delayed start)
- **Transaction**: TransactionAction (commit on all success, rollback on any failure)

### Event — Event Definition

```cpp
struct Event {
    using ID = int;
    ID id = 0;
    const void *extra = nullptr;  //! Attached data pointer
};
```

Supports creation from enum types: `Event(MyEvent::kTimeout, &data)`

## Usage Examples

### State Machine — Simple Switch

> See unit test cases in `modules/flow/` for complete examples

```cpp
enum State { kOff = 1, kOn = 2 };
enum Event { kToggle = 10 };

StateMachine sm;

//! Create two states
sm.newState(kOff, [](Event) { LogInfo("enter OFF"); }, [](Event) { LogInfo("exit OFF"); });
sm.newState(kOn, [](Event) { LogInfo("enter ON"); }, [](Event) { LogInfo("exit ON"); });

//! Add transition routes: switch to the other state upon receiving Toggle event from any state
sm.addRoute(kOff, kToggle, kOn, nullptr, nullptr);
sm.addRoute(kOn, kToggle, kOff, nullptr, nullptr);

sm.start();            //! Start from the first state kOff
sm.run(Event(kToggle)); //! OFF → ON
sm.run(Event(kToggle)); //! ON → OFF
```

### State Machine — Conditional Route

```cpp
//! Route with condition judgment: transition only when guard returns true
sm.addRoute(kIdle, kRequest, kBusy,
    [](Event ev) { return /* some condition */; },
    nullptr
);
```

### Sub State Machine Nesting

```cpp
StateMachine outer_sm;
StateMachine inner_sm;

//! Inner state machine definition
inner_sm.newState(kInnerA, ...);
inner_sm.newState(kInnerB, ...);

//! Attach the inner state machine to a state of the outer state machine
outer_sm.newState(kOuterState, ...);
outer_sm.setSubStateMachine(kOuterState, &inner_sm);
```

### Behavior Tree — Sequential Execution

> See unit test cases in `modules/flow/` for complete examples

```cpp
//! SequentialAction: execute multiple sub-actions in order
//! Sub-action A completes then automatically starts B, B completes then starts C
//! Any failure causes the overall action to fail
```

### Export Graphviz Diagram

```cpp
Json js;
sm.toJson(js);  //! Export state machine as JSON, can be used for visualization
//! Use to_graphviz.h to convert to Graphviz format
```

## Common Scenarios

1. **Device state management**: e.g., IoT device idle → running → fault → recovery state transitions
2. **Protocol state machine**: e.g., TCP connection states, HTTP request processing states
3. **Business flow orchestration**: e.g., order creation → payment → shipping → completion, rollback on failure
4. **Conditional branching**: Select different execution paths based on sensor data
5. **Retry mechanism**: Use RetryAction to automatically retry on failure

## Important Notes

1. **StateID 0 is the terminated state**: The state machine automatically terminates upon reaching state 0, no additional handling needed
2. **Sub state machine lifetime**: The sub state machine passed to setSubStateMachine() must have a longer lifetime than the parent state machine
3. **Action finish/block**: finish() indicates normal completion (success or failure), block() indicates pausing to wait for an external condition
4. **Event.extra pointer**: The data pointed to by the extra pointer must remain valid during event handling
5. **toJson export**: The state machine can be exported as JSON for debugging and visualization

## Related Modules

- **event**: Run state machines and actions based on Loop
- **util**: Action uses Variables to store variables
- **base**: Provides Json, Log and other infrastructure
