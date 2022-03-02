#include "loop.h"

#include <ev.h>

#include "fd_event.h"
#include "timer_event.h"
#include "signal_event.h"

namespace tbox {
namespace event {

LibevLoop::LibevLoop() :
    sp_ev_loop_(ev_loop_new()),
    sp_exit_timer_(nullptr)
{ }

LibevLoop::~LibevLoop()
{
    cleanupDeferredTasks();

    delete sp_exit_timer_;
    ev_loop_destroy(sp_ev_loop_);
}

void LibevLoop::runLoop(Mode mode)
{
    int flags = 0;
    if (mode == Mode::kOnce)
        flags |= EVRUN_ONCE;

    runThisBeforeLoop();
    ev_run(sp_ev_loop_, flags);
    runThisAfterLoop();
}

void LibevLoop::exitLoop(const std::chrono::milliseconds &wait_time)
{
    if (wait_time.count() == 0) {
        ev_break(sp_ev_loop_, EVBREAK_ALL);
    } else {
        sp_exit_timer_ = newTimerEvent();
        sp_exit_timer_->initialize(wait_time, Event::Mode::kOneshot);
        sp_exit_timer_->setCallback(std::bind(&LibevLoop::onExitTimeup, this));
        sp_exit_timer_->enable();
    }
}

void LibevLoop::onExitTimeup()
{
    sp_exit_timer_->disable();
    ev_break(sp_ev_loop_, EVBREAK_ALL);
}

FdEvent* LibevLoop::newFdEvent()
{
    return new LibevFdEvent(this);
}

TimerEvent* LibevLoop::newTimerEvent()
{
    return new LibevTimerEvent(this);
}

}
}
