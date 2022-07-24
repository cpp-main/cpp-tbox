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
    auto now = steady_clock::now();

    if (stoped_)
        return;

    auto cost = now - start_time_point_;
    LogPrintfFunc("TC", func_name_, file_name_, line_, LOG_LEVEL_TRACE, true, "timecost: %llu ns", cost.count());
    stoped_ = true;
}

}
}
