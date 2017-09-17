#include "loop.h"

#include <ev.h>

#include "fd_item.h"
#include "timer_item.h"
#include "signal_item.h"

namespace tbox {
namespace event {

LibevLoop::LibevLoop() :
    sp_ev_loop_(ev_loop_new()),
    sp_exit_timer_(nullptr)
{ }

LibevLoop::~LibevLoop()
{
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

void LibevLoop::exitLoop(const Timespan &wait_time)
{
    if (wait_time.isZero()) {
        ev_break(sp_ev_loop_, EVBREAK_ALL);
    } else {
        sp_exit_timer_ = newTimerItem();
        sp_exit_timer_->initialize(wait_time, Item::Mode::kOneshot);
        sp_exit_timer_->setCallback(std::bind(&LibevLoop::onExitTimeup, this));
        sp_exit_timer_->enable();
    }
}

void LibevLoop::onExitTimeup()
{
    sp_exit_timer_->disable();
    ev_break(sp_ev_loop_, EVBREAK_ALL);
}

FdItem* LibevLoop::newFdItem()
{
    return new LibevFdItem(this);
}

TimerItem* LibevLoop::newTimerItem()
{
    return new LibevTimerItem(this);
}

SignalItem* LibevLoop::newSignalItem()
{
    return new LibevSignalItem(this);
}

}
}