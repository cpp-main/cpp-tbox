#include <cstring>
#include <cassert>
#include <cstdlib>
#include <chrono>
#include <sys/time.h>

#include "cron_timer.h"
#include "tbox/event/timer_event.h"
#include "tbox/event/loop.h"
#include "ccronexpr.h"

namespace tbox
{
namespace event
{

CrontabTimer::CrontabTimer(Loop *loop) :
    wp_loop_(loop),
    sp_timer_ev_(loop->newTimerEvent())
{
    assert(sp_timer_ev_ != nullptr);
    sp_cron_expr_ = malloc(sizeof(cron_expr));
    assert(sp_cron_expr_ != nullptr);
    memset(sp_cron_expr_, 0, sizeof(cron_expr));
}

CrontabTimer::~CrontabTimer()
{
    free(sp_cron_expr_);
    delete sp_timer_ev_;
}

bool CrontabTimer::isEnabled() const
{
    return sp_timer_ev_->isEnabled();
}

bool CrontabTimer::enable()
{
    return setNextAlarm();
}

bool CrontabTimer::disable()
{
    return sp_timer_ev_->disable();
}

bool CrontabTimer::initialize(const std::string &crontab_str, Callback cb)
{
    const char *error_str = nullptr;
    memset(sp_cron_expr_, 0, sizeof(cron_expr));

    // check validity of crontab str
    cron_parse_expr(crontab_str.c_str(), static_cast<cron_expr *>(sp_cron_expr_), &error_str);
    if (error_str != nullptr) { // Invalid expression.
        return false;
    }

    cb_ = cb;
    sp_timer_ev_->setCallback([this] { this->onTimeExpired(); });

    return true;
}

void CrontabTimer::cleanup()
{
    cb_ = nullptr;
}

void CrontabTimer::refresh()
{
    if (sp_timer_ev_->isEnabled())
        setNextAlarm();
}

void CrontabTimer::onTimeExpired()
{
    setNextAlarm();

    if (cb_)
        cb_();
}

bool CrontabTimer::setNextAlarm()
{
    struct timeval now_tv;
    gettimeofday(&now_tv, nullptr);
    auto next_ts = cron_next(static_cast<cron_expr *>(sp_cron_expr_), now_tv.tv_sec);
    auto duration_s = next_ts - now_tv.tv_sec;
    auto duration_ms = duration_s * 1000 - now_tv.tv_usec / 1000;

    sp_timer_ev_->initialize(std::chrono::milliseconds(duration_ms), Mode::kOneshot);
    return sp_timer_ev_->enable();
}

}
}
