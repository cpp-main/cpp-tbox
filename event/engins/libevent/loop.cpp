#include "loop.h"

#include <chrono>
#include <event2/event.h>

#include "fd_event.h"
#include "timer_event.h"
#include "common.h"

namespace tbox {
namespace event {

LibeventLoop::LibeventLoop() :
    sp_event_base_(event_base_new())
{ }

LibeventLoop::~LibeventLoop()
{
    cleanupDeferredTasks();

    event_base_free(sp_event_base_);
    sp_event_base_ = NULL;
}

void LibeventLoop::runLoop(Mode mode)
{
    int flags = 0;
    if (mode == Mode::kOnce)
        flags |= EVLOOP_ONCE;

    runThisBeforeLoop();
    event_base_loop(sp_event_base_, flags);
    runThisAfterLoop();
}

void LibeventLoop::exitLoop(const std::chrono::milliseconds &wait_time)
{
    struct timeval tv = Chrono2Timeval(wait_time);
    event_base_loopexit(sp_event_base_, &tv);
}

FdEvent* LibeventLoop::newFdEvent()
{
    return new LibeventFdEvent(this);
}

TimerEvent* LibeventLoop::newTimerEvent()
{
    return new LibeventTimerEvent(this);
}

}
}
