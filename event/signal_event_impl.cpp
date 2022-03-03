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
    sigset_.insert(signo);
    mode_ = mode;

    is_inited_ = true;
    return true;
}

bool SignalEventImpl::initialize(const std::set<int> &sigset, Mode mode)
{
    sigset_ = sigset;
    mode_ = mode;

    is_inited_ = true;
    return true;
}

bool SignalEventImpl::enable()
{
    if (is_inited_) {
        for (int signo : sigset_) {
            if (!wp_loop_->subscribeSignal(signo, this)) {
                return false;
            }
        }
    }
    is_enabled_ = true;
    return true;
}

bool SignalEventImpl::disable()
{
    if (is_enabled_) {
        for (int signo : sigset_) {
            if (!wp_loop_->unsubscribeSignal(signo, this)) {
                return false;
            }
        }
    }
    is_enabled_ = false;
    return true;
}

Loop* SignalEventImpl::getLoop() const
{
    return wp_loop_;
}

void SignalEventImpl::onSignal(int signo)
{
    if (mode_ == Mode::kOneshot)
        disable();

    if (cb_)
        cb_(signo);
}

}
}
