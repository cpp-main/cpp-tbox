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
#ifndef TBOX_EVENT_TIMER_ITEM_H_20170627
#define TBOX_EVENT_TIMER_ITEM_H_20170627

#include <functional>
#include <chrono>

#include "event.h"

namespace tbox {
namespace event {

class TimerEvent : public Event {
  public:
    using Event::Event;

    virtual bool initialize(const std::chrono::milliseconds &time_span, Mode mode) = 0;

    using CallbackFunc = std::function<void ()>;
    virtual void setCallback(CallbackFunc &&cb) = 0;

  public:
    virtual ~TimerEvent() { }
};

}
}

#endif //TBOX_EVENT_TIMER_ITEM_H_20170627
