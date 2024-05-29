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
#include "recorder.h"

#include <tbox/util/timestamp.h>
#include "sink.h"

namespace tbox {
namespace trace {

Recorder::Recorder(const char *name)
  : name_(name)
{
    if (Sink::GetInstance().isEnabled())
        start_ts_us_ = util::GetCurrentMicrosecondsFrom1970();
}

Recorder::~Recorder()
{
    stop();
}

void Recorder::stop()
{
    if (start_ts_us_ == 0)
        return;

    auto end_ts_us = util::GetCurrentMicrosecondsFrom1970();
    auto duration_us = end_ts_us - start_ts_us_;

    Sink::GetInstance().commitRecord(name_, end_ts_us, duration_us);
    start_ts_us_ = 0;
}

}
}
