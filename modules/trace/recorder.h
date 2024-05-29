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
#ifndef TBOX_TRACE_RECORDER_H_20240525
#define TBOX_TRACE_RECORDER_H_20240525

#include <cstdint>

namespace tbox {
namespace trace {

class Recorder {
  public:
    Recorder(const char *name);
    ~Recorder();

    void stop();

  private:
    const char *name_;
    uint64_t start_ts_us_ = 0;
};

}
}

//! 区域记录器
#define _RECORDER_1(func,line)  tbox::trace::Recorder _trace_recorder_at_##line(func)
#define _RECORDER_0(func,line)  _RECORDER_1(func,line)
#define RECORD_SCOPE()  _RECORDER_0(__PRETTY_FUNCTION__, __LINE__)

//! 有名记录器
#define _NAMED_RECORDER_0(func,line,name) tbox::trace::Recorder _trace_recorder_##name(func " " ##name)
#define RECORD_START(name)  _NAMED_RECORDER_0(__PRETTY_FUNCTION__, __LINE__, name)
#define RECORD_STOP(name)   _trace_recorder_##name.stop()

#endif //TBOX_TRACE_RECORDER_H_20240525
