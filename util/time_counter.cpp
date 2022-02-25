#include "time_counter.h"
#include <tbox/base/log.h>

namespace tbox {
namespace util {

using namespace std::chrono;

TimeCounter::TimeCounter(const char *file_name, const char *func_name, int line) :
    file_name_(file_name),
    func_name_(func_name),
    line_(line)
{
    start_time_point_ = steady_clock::now();
}

TimeCounter::~TimeCounter()
{
    stop();
}

void TimeCounter::stop()
{
    if (stoped_)
        return;

    auto cost_us = duration_cast<microseconds>(steady_clock::now() - start_time_point_).count();
    LogPrintfFunc("TC", func_name_, file_name_, line_, LOG_LEVEL_TRACE, "timecost: %u us", cost_us);
    stoped_ = true;
}

}
}
