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
#ifndef TBOX_EVENT_FD_ITEM_H_20170627
#define TBOX_EVENT_FD_ITEM_H_20170627

#include <functional>

#include "event.h"

namespace tbox {
namespace event {

class FdEvent : public Event {
  public:
    enum EventTypes {
        kReadEvent   = 0x01,    //!< 可读事件
        kWriteEvent  = 0x02,    //!< 可写事件
        kExceptEvent = 0x04,    //!< 异常事件
    };

    using Event::Event;

    virtual bool initialize(int fd, short events, Mode mode) = 0;

    using CallbackFunc = std::function<void (short events)>;
    virtual void setCallback(CallbackFunc &&cb) = 0;

  public:
    virtual ~FdEvent() { }
};

}
}

#endif //TBOX_EVENT_FD_ITEM_H_20170627
