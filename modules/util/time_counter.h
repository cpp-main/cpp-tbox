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
#ifndef TBOX_UTIL_TIME_COUNTER_H_20220225
#define TBOX_UTIL_TIME_COUNTER_H_20220225

#include <chrono>

namespace tbox {
namespace util {

class TimeCounter {
  public:
    TimeCounter();

    //! 开始计时
    void start();

    //! 获取㳿逝的时间，单位纳秒
    uint64_t elapsed() const;

    //! 打印已流逝的时间到终端
    void print(const char *tag, uint64_t threshold_ns = 0) const;

  private:
    std::chrono::steady_clock::time_point start_time_point_;
};

class CpuTimeCounter {
  public:
    CpuTimeCounter();

    //! 开始计时
    void start();

    //! 获取㳿逝的时间，单位纳秒
    uint64_t elapsed() const;

    //! 打印已流逝的时间到终端
    void print(const char *tag, uint64_t threshold_ns = 0) const;

  private:
    struct timespec start_time_;
};

class FixedTimeCounter {
  public:
    FixedTimeCounter(const char *file_name, const char *func_name, int line,
                     uint64_t threshold_ns = 0);
    ~FixedTimeCounter();

    void stop();

  private:
    const char *file_name_;
    const char *func_name_;
    int line_;
    std::chrono::nanoseconds threshold_;
    std::chrono::steady_clock::time_point start_time_point_;
    bool stoped_ = false;
};

}
}

//! 无名计时器，用行号命名
#define _TimeCounter_1(file,func,line) tbox::util::FixedTimeCounter _timer_counter_at_##line(file,func,line)
#define _TimeCounter_0(file,func,line) _TimeCounter_1(file,func,line)
#define SetTimeCounter() _TimeCounter_0(__FILE__, __func__, __LINE__)

//! 有名计时器
#define _NamedTimeCounter_0(file,func,line,name) tbox::util::FixedTimeCounter _timer_counter_##name(file,func,line)
#define SetNamedTimeCounter(name) _NamedTimeCounter_0(__FILE__, __func__, __LINE__, name)
#define StopNamedTimeCounter(name) _timer_counter_##name.stop()

//! 无名有阈值计时器，用行号命名
#define _TimeCounterWithThreshold_1(file,func,line,threshold) tbox::util::FixedTimeCounter _timer_counter_at_##line(file,func,line,threshold)
#define _TimeCounterWithThreshold_0(file,func,line,threshold) _TimeCounterWithThreshold_1(file,func,line,threshold)
#define SetTimeCounterWithThreshold(threshold) _TimeCounterWithThreshold_0(__FILE__, __func__, __LINE__, threshold)

//! 有名有阈值计时器
#define _NamedTimeCounterWithThreshold_0(file,func,line,name,threshold) tbox::util::FixedTimeCounter _timer_counter_with_threshold_##name(file,func,line,threshold)
#define SetNamedTimeCounterWithThreshold(name,threshold) _NamedTimeCounterWithThreshold_0(__FILE__, __func__, __LINE__, name, threshold)
#define StopNamedTimeCounterWithThreshold(name) _timer_counter_with_threshold_##name.stop()

#endif //TBOX_UTIL_TIME_COUNTER_H_20220225
