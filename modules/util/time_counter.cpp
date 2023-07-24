/*
 *     .============.
 *    //  M A K E  / \
 *   //  C++ DEV  /   \
 *  //  E A S Y  /  \/ \
 * ++ ----------.  \/\  .
 *  \\     \     \ /\  /
 *   \\     \     \   /
 *    \\     \     \ /
 *     -============'
 *
 * Copyright (c) 2018 Hevake and contributors, all rights reserved.
 *
 * This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
 * Use of this source code is governed by MIT license that can be found
 * in the LICENSE file in the root of the source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */
#include "time_counter.h"

#define USE_PRINTF 1

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

void TimeCounter::print(const char *tag, uint64_t threshold_ns) const
{
    auto ns_count = elapsed();
    if (ns_count <= threshold_ns)
        return;

#if USE_PRINTF
    printf("TIME_COST: %8" PRIu64 ".%03" PRIu64 " us at [%s] \n",
           ns_count / 1000, ns_count % 1000, tag);
#else
    cout << "TIME_COST: " << setw(8) << ns_count / 1000
         << '.' << setw(3) << setfill('0') << ns_count % 1000 << setfill(' ')
         << " us on [" << tag << ']' << endl;
#endif
}

CpuTimeCounter::CpuTimeCounter()
{
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_time_);
}

void CpuTimeCounter::start()
{
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_time_);
}

uint64_t CpuTimeCounter::elapsed() const
{
    const uint64_t nsec_per_sec = 1000000000;
    struct timespec end_time;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end_time);
    uint64_t ns_count = (end_time.tv_sec - start_time_.tv_sec) * nsec_per_sec;
    ns_count += end_time.tv_nsec;
    ns_count -= start_time_.tv_nsec;

    return ns_count;
}

void CpuTimeCounter::print(const char *tag, uint64_t threshold_ns) const
{
    auto ns_count = elapsed();
    if (ns_count <= threshold_ns)
        return;

#if USE_PRINTF
    printf("CPU_TIME_COST: %8" PRIu64 ".%03" PRIu64 " us at [%s] \n",
           ns_count / 1000, ns_count % 1000, tag);
#else
    cout << "CPU_TIME_COST: " << setw(8) << ns_count / 1000
         << '.' << setw(3) << setfill('0') << ns_count % 1000 << setfill(' ')
         << " us on [" << tag << ']' << endl;
#endif
}

/////////////////////
// FixedTimeCounter
/////////////////////

FixedTimeCounter::FixedTimeCounter(const char *file_name, const char *func_name, int line, uint64_t threshold_ns) :
    file_name_(file_name),
    func_name_(func_name),
    line_(line),
    threshold_(std::chrono::nanoseconds(threshold_ns))
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
    cout << "TIME_COST: " << setw(8) << ns_count / 1000
         << '.' << setw(3) << setfill('0') << ns_count % 1000 << setfill(' ')
         << " us at " << func_name_ << "() in "
         << Basename(file_name_) << ':' << line_ << endl;
#endif

}

}
}

#undef USE_PRINTF

