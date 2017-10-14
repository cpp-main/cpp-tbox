#include "timer_item.h"

#include <cassert>

#include "loop.h"
#include <tbox/base/log.h>

namespace tbox {
namespace event {

LibevTimerItem::LibevTimerItem(LibevLoop *wp_loop) :
    wp_loop_(wp_loop),
    is_inited_(false),
    mode_(Mode::kOneshot),
    cb_level_(0)
{
    memset(&timer_ev_, 0, sizeof(timer_ev_));
}

LibevTimerItem::~LibevTimerItem()
{
    assert(cb_level_ == 0);

    disable();
}

namespace {
double TimevalToDouble(const Timespan &v)
{
    struct timeval tv = v;
    return double(tv.tv_sec) + tv.tv_usec * 0.000001;
}

}

bool LibevTimerItem::initialize(const Timespan &interval, Mode mode)
{
    disable();

    timer_ev_.active = timer_ev_.pending = 0;
    timer_ev_.priority = 0;
    timer_ev_.cb = LibevTimerItem::OnEventCallback;
    timer_ev_.data = this;

    interval_ = interval;
    mode_ = mode;

    is_inited_ = true;
    return true;
}

void LibevTimerItem::setCallback(const CallbackFunc &cb)
{
    cb_ = cb;
}

bool LibevTimerItem::isEnabled() const
{
    if (!is_inited_)
        return false;

    return timer_ev_.active;
}

bool LibevTimerItem::enable()
{
    if (!is_inited_) {
        //! 没有初始化，是不能直接enable的
        LogErr("can't enable() before initialize()");
        return false;
    }

    if (isEnabled())
        return true;

    double d_interval = TimevalToDouble(interval_);
    if (mode_ == Mode::kOneshot) {
        timer_ev_.repeat = 0;
        timer_ev_.at = d_interval;
    } else {
        timer_ev_.repeat = timer_ev_.at = d_interval;
    }

    ev_timer_start(wp_loop_->getEvLoopPtr(), &timer_ev_);

    return true;
}

bool LibevTimerItem::disable()
{
    if (!is_inited_)
        return false;

    if (!isEnabled())
        return true;

    ev_timer_stop(wp_loop_->getEvLoopPtr(), &timer_ev_);

    return true;
}

void LibevTimerItem::OnEventCallback(struct ev_loop*, ev_timer *p_w, int events)
{
    assert(p_w != NULL);

    LibevTimerItem *pthis = static_cast<LibevTimerItem*>(p_w->data);
    pthis->onEvent();
}

void LibevTimerItem::onEvent()
{
#ifdef  ENABLE_STAT
    using namespace std::chrono;
    auto start = steady_clock::now();
#endif

    if (cb_) {
        ++cb_level_;
        cb_();
        --cb_level_;

    } else {
        LogWarn("you should specify event callback by setCallback()");
    }

#ifdef  ENABLE_STAT
    uint64_t cost_us = duration_cast<microseconds>(steady_clock::now() - start).count();
    wp_loop_->recordTimeCost(cost_us);
#endif
}

}
}
