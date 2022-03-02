#include "signal_event_impl.h"
#include "common_loop.h"
#include <tbox/base/log.h>

namespace tbox {
namespace event {

SignalEventImpl::SignalEventImpl(CommonLoop *wp_loop) :
    wp_loop_(wp_loop)
{ }

SignalEventImpl::~SignalEventImpl()
{
    disable();
}

bool SignalEventImpl::initialize(int signo, Mode mode)
{
    signo_ = signo;
    mode_ = mode;

    is_inited_ = true;
    return true;
}

bool SignalEventImpl::enable()
{
    if (is_inited_) {
        if (wp_loop_->subscribeSignal(signo_, this)) {
            is_enabled_ = true;
            return true;
        }
    }
    return false;
}

bool SignalEventImpl::disable()
{
    if (is_enabled_) {
        if (wp_loop_->unsubscribeSignal(signo_, this)) {
            is_enabled_ = false;
            return true;
        }
    }
    return false;
}

Loop* SignalEventImpl::getLoop() const
{
    return wp_loop_;
}

void SignalEventImpl::onSignal(int /*signo*/)
{
    if (mode_ == Mode::kOneshot)
        disable();

    if (cb_)
        cb_();
}

}
}
