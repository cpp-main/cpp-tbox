#include "timers.h"
#include <tbox/base/log.h>

namespace tbox::eventx {

Timers::Timers(event::Loop *wp_loop)
{
    LogUndo();
}

Timers::~Timers()
{
    LogUndo();
}

Timers::Token Timers::doEvery(const Milliseconds &m_sec, const Callback &cb)
{
    LogUndo();
    return Token();
}

Timers::Token Timers::doAfter(const Milliseconds &m_sec, const Callback &cb)
{
    LogUndo();
    return Token();
}

Timers::Token Timers::doAt(const TimePoint &time_point, const Callback &cb)
{
    LogUndo();
    return Token();
}

Timers::Token Timers::AddCron(const std::string &cron_exp, const Callback &cb)
{
    LogUndo();
    return Token();
}

bool Timers::cancel(const Token &token)
{
    LogUndo();
    return false;
}

}
