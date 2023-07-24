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
#ifndef TBOX_FLOW_EVENT_SUBSCRIBER_H_20221001
#define TBOX_FLOW_EVENT_SUBSCRIBER_H_20221001

#include "event.h"

namespace tbox {
namespace flow {

class EventSubscriber {
  public:
    virtual bool onEvent(Event event) = 0;

  protected:
    virtual ~EventSubscriber() { }
};

}
}

#endif //TBOX_FLOW_EVENT_SUBSCRIBER_H_20221001
