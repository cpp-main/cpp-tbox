#include "loop.h"

#include <event2/event.h>

#include "fd_item.h"
#include "timer_item.h"
#include "signal_item.h"

namespace tbox {
namespace event {

LibeventLoop::LibeventLoop() :
    sp_event_base_(event_base_new())
{ }

LibeventLoop::~LibeventLoop()
{
    event_base_free(sp_event_base_);
    sp_event_base_ = NULL;
}

void LibeventLoop::runLoop(Mode mode)
{
    int flags = 0;
    if (mode == Mode::kOnce)
        flags = EVLOOP_ONCE;

    runThisBeforeLoop();
    event_base_loop(sp_event_base_, flags);
    runThisAfterLoop();
}

void LibeventLoop::exitLoop(const Timespan &wait_time)
{
    struct timeval tv = wait_time;
    event_base_loopexit(sp_event_base_, &tv);
}

FdItem* LibeventLoop::newFdItem()
{
    return new LibeventFdItem(this);
}

TimerItem* LibeventLoop::newTimerItem()
{
    return new LibeventTimerItem(this);
}

SignalItem* LibeventLoop::newSignalItem()
{
    return new LibeventSignalItem(this);
}

}
}
