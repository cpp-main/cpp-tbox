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
* Copyright (c) 2024 Hevake and contributors, all rights reserved.
*
* This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
* Use of this source code is governed by MIT license that can be found
* in the LICENSE file in the root of the source tree. All contributing
* project authors may be found in the CONTRIBUTORS.md file in the root
* of the source tree.
*/
#include"recorder.h"

#include <sys/time.h>

namespace tbox {
namespace trace {

//!定义函数的弱引用
void __attribute((weak)) CommitRecordFunc(
   const char *name, const char *module, uint32_t line,
   uint64_t end_timepoint_us, uint64_t duration_us
);

namespace{
uint64_t GetCurrentUTCMicroseconds()
{
   struct timeval tv;
   struct timezone tz;
   gettimeofday(&tv, &tz);
   return tv.tv_sec * 1000000 + tv.tv_usec;
}
}

Recorder::Recorder(const char *name, const char *module, uint32_t line, bool start_now)
 : name_(name)
 , module_(module)
 , line_(line)
{
   if (start_now)
       start();
}

Recorder::~Recorder()
{
   stop();
}

void Recorder::start()
{
   if (CommitRecordFunc)
       start_ts_us_ = GetCurrentUTCMicroseconds();
}

void Recorder::stop()
{
   if (start_ts_us_ == 0)
       return;

   auto end_ts_us = GetCurrentUTCMicroseconds();
   auto duration_us = end_ts_us - start_ts_us_;

   if (CommitRecordFunc)
       CommitRecordFunc(name_, module_, line_, end_ts_us, duration_us);

   start_ts_us_ = 0;
}

void RecordEvent(const char *name, const char *module, uint32_t line)
{
   if (CommitRecordFunc)
       CommitRecordFunc(name, module, line, GetCurrentUTCMicroseconds(), 0);
}

}
}
