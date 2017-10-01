#include "signal_item.h"

#include <cassert>

#include "loop.h"
#include <tbox/log.h>

namespace tbox {
namespace event {

LibevSignalItem::LibevSignalItem(LibevLoop *wp_loop) :
    wp_loop_(wp_loop),
    is_inited_(false),
    is_stop_after_trigger_(false),
    cb_level_(0)
{
    memset(&signal_ev_, 0, sizeof(signal_ev_));
}

LibevSignalItem::~LibevSignalItem()
{
    assert(cb_level_ == 0);
    disable();
}

bool LibevSignalItem::initialize(int signum, Mode mode)
{
    disable();

    signal_ev_.active = signal_ev_.pending = 0;
    signal_ev_.priority = 0;
    signal_ev_.cb = LibevSignalItem::OnEventCallback;
    signal_ev_.data = this;

    signal_ev_.signum = signum;

    if (mode == Mode::kOneshot) //! 如果是单次有效的，需要设置标记，使之在触发后停止事件
        is_stop_after_trigger_ = true;

    is_inited_ = true;
    return true;
}

void LibevSignalItem::setCallback(const CallbackFunc &cb)
{
    cb_ = cb;
}

bool LibevSignalItem::isEnabled() const
{
    if (!is_inited_)
        return false;

    return signal_ev_.active;
}

bool LibevSignalItem::enable()
{
    if (!is_inited_) {
        LogErr("can't enable() before initialize()");
        return false;
    }

    if (isEnabled())
        return true;

    ev_signal_start(wp_loop_->getEvLoopPtr(), &signal_ev_);

    return true;
}

bool LibevSignalItem::disable()
{
    if (!is_inited_)
        return false;

    if (!isEnabled())
        return true;

    ev_signal_stop(wp_loop_->getEvLoopPtr(), &signal_ev_);

    return true;
}

void LibevSignalItem::OnEventCallback(struct ev_loop*, ev_signal *p_w, int events)
{
    assert(p_w != NULL);

    LibevSignalItem *pthis = static_cast<LibevSignalItem*>(p_w->data);
    pthis->onEvent();
}

void LibevSignalItem::onEvent()
{
#ifdef  ENABLE_STAT
    using namespace std::chrono;
    auto start = steady_clock::now();
#endif

    if (cb_) {
        ++cb_level_;
        cb_();
        --cb_level_;

        if (is_stop_after_trigger_)
            disable();

    } else {
        LogErr("you should specify event callback by setCallback()");
    }

#ifdef  ENABLE_STAT
    uint64_t cost_us = duration_cast<microseconds>(steady_clock::now() - start).count();
    wp_loop_->recordTimeCost(cost_us);
#endif
}

}
}
