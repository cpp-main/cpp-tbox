#include "signal_event_impl.h"
#include "common_loop.h"
#include <tbox/base/log.h>

namespace tbox {
namespace event {

SignalEventImpl::SignalEventImpl(CommonLoop *wp_loop) :
    wp_loop_(wp_loop)
{
    LogUndo();
}

SignalEventImpl::~SignalEventImpl()
{
    LogUndo();
}

bool SignalEventImpl::initialize(int signum, Mode mode)
{
    LogUndo();
    return false;
}

bool SignalEventImpl::isEnabled() const
{
    LogUndo();
    return false;
}

bool SignalEventImpl::enable()
{
    LogUndo();
    return false;
}

bool SignalEventImpl::disable()
{
    LogUndo();
    return false;
}

Loop* SignalEventImpl::getLoop() const
{
    return wp_loop_;
}

}
}
