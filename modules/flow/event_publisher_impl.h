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
#ifndef TBOX_FLOW_EVENT_PUBLISHER_H_20221002
#define TBOX_FLOW_EVENT_PUBLISHER_H_20221002

#include <vector>
#include "event_publisher.h"

namespace tbox {
namespace flow {

class EventPublisherImpl : public EventPublisher {
  public:
    virtual void subscribe(EventSubscriber *subscriber) override;
    virtual void unsubscribe(EventSubscriber *subscriber) override;
    virtual void publish(Event event) override;

  private:
    std::vector<EventSubscriber*> subscriber_vec_;
    std::vector<EventSubscriber*> tmp_vec_;
};



}
}

#endif //TBOX_FLOW_EVENT_PUBLISHER_H_20221002
