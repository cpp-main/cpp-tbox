#include "crontab_timer.h"

#include <cstring>
#include <cassert>
#include <sys/time.h>

#include <tbox/base/log.h>
#include <tbox/event/timer_event.h>
#include <tbox/event/loop.h>
#include "ccronexpr.h"

namespace tbox {
namespace timer {

CrontabTimer::CrontabTimer(event::Loop *loop) :
    wp_loop_(loop),
    sp_timer_ev_(loop->newTimerEvent())
{
    assert(sp_timer_ev_ != nullptr);
    sp_timer_ev_->setCallback([this] { onTimeExpired(); });

    sp_cron_expr_ = malloc(sizeof(cron_expr));
    assert(sp_cron_expr_ != nullptr);
    memset(sp_cron_expr_, 0, sizeof(cron_expr));
}

CrontabTimer::~CrontabTimer()
{
    assert(cb_level_ == 0);

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

bool CrontabTimer::initialize(const std::string &crontab_expr, int timezone_offset_minutes)
{
    const char *error_str = nullptr;
    memset(sp_cron_expr_, 0, sizeof(cron_expr));

    // check validity of crontab str
    cron_parse_expr(crontab_expr.c_str(), static_cast<cron_expr *>(sp_cron_expr_), &error_str);
    if (error_str != nullptr) { // Invalid expression.
        LogWarn("crontab_expr error:%s", *error_str);
        return false;
    }

    timezone_offset_seconds_ = timezone_offset_minutes * 60;
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

    ++cb_level_;
    if (cb_)
        cb_();
    --cb_level_;
}

bool CrontabTimer::setNextAlarm()
{
    struct timeval tv;
    int ret = gettimeofday(&tv, nullptr);
    if (ret != 0) {
        LogWarn("gettimeofday() fail, ret:%d", ret);
        return false;
    }

    auto curr_local_ts = tv.tv_sec + timezone_offset_seconds_;
    auto next_local_ts = cron_next(static_cast<cron_expr *>(sp_cron_expr_), curr_local_ts);
    auto duration_s = next_local_ts - curr_local_ts;
    auto duration_ms = duration_s * 1000 - tv.tv_usec / 1000;

    sp_timer_ev_->initialize(std::chrono::milliseconds(duration_ms), event::Event::Mode::kOneshot);
    return sp_timer_ev_->enable();
}

}
}
