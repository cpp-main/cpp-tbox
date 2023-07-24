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
#ifndef TBOX_EVENT_SIGNAL_ITEM_H_20170627
#define TBOX_EVENT_SIGNAL_ITEM_H_20170627

#include <signal.h>
#include <functional>
#include <set>

#include "event.h"

namespace tbox {
namespace event {

class SignalEvent : public Event {
  public:
    using Event::Event;

    virtual bool initialize(int signum, Mode mode) = 0;
    virtual bool initialize(const std::set<int> &sigset, Mode mode) = 0;
    virtual bool initialize(const std::initializer_list<int> &sigset, Mode mode) = 0;

    using CallbackFunc = std::function<void (int)>;
    virtual void setCallback(CallbackFunc &&cb) = 0;

  public:
    virtual ~SignalEvent() { }
};

}
}

#endif //TBOX_EVENT_SIGNAL_ITEM_H_20170627
