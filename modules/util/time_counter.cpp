#include "time_counter.h"

#define USE_PRINTF 0

#if USE_PRINTF
# include <cstdio>
# include <cinttypes>
#else
# include <iostream>
# include <iomanip>
#endif

namespace tbox {
namespace util {

using namespace std;
using namespace std::chrono;

namespace {
const char* Basename(const char *full_path)
{
    const char *p_last = full_path;
    if (p_last != nullptr) {
        for (const char *p = full_path; *p; ++p) {
            if (*p == '/')
                p_last = p + 1;
        }
    }
    return p_last;
}
}

TimeCounter::TimeCounter()
    : start_time_point_(steady_clock::now())
{ }

void TimeCounter::start()
{
    start_time_point_ = steady_clock::now();
}

uint64_t TimeCounter::elapsed() const
{
    auto cost = steady_clock::now() - start_time_point_;
    return cost.count();
}

void TimeCounter::print(const char *tag) const
{
    auto ns_count = elapsed();

#if USE_PRINTF
    printf("TIME_COST: %8" PRIu64 ".%03" PRIu64 " us at [%s] \n",
           ns_count / 1000, ns_count % 1000, tag);
#else
    cout << "TIME_COST: " << setw(8) << ns_count / 1000 << '.' << ns_count % 1000 << " us on [" << tag << ']' << endl;
#endif
}

/////////////////////
// FixedTimeCounter
/////////////////////

FixedTimeCounter::FixedTimeCounter(const char *file_name, const char *func_name, int line,
                         std::chrono::nanoseconds threshold) :
    file_name_(file_name),
    func_name_(func_name),
    line_(line),
    threshold_(threshold)
{
    start_time_point_ = steady_clock::now();
}

FixedTimeCounter::~FixedTimeCounter()
{
    stop();
}

void FixedTimeCounter::stop()
{
    if (stoped_)
        return;
    stoped_ = true;

    auto cost = steady_clock::now() - start_time_point_;
    if (cost < threshold_)
        return;

    auto ns_count = cost.count();

#if USE_PRINTF
    printf("TIME_COST: %8" PRIu64 ".%03" PRIu64 " us at %s() in %s:%u\n",
           ns_count / 1000, ns_count % 1000,
           func_name_, Basename(file_name_), line_);
#else
    cout << "TIME_COST: " << setw(8) << ns_count / 1000 << '.' << ns_count % 1000 << " us at"
         << func_name_ << "() in " << Basename(file_name_) << ':' << line_ << endl;
#endif

}

}
}

#undef USE_PRINTF

