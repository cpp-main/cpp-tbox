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
#ifndef TBOX_FLOW_EVENT_ACTION_H_20221105
#define TBOX_FLOW_EVENT_ACTION_H_20221105

#include "../action.h"
#include "../event_subscriber.h"
#include "../event_publisher.h"

namespace tbox {
namespace flow {

class EventAction : public Action,
                    public EventSubscriber {
  public:
    explicit EventAction(event::Loop &loop, const std::string &type, EventPublisher &pub);
    virtual ~EventAction();

  protected:
    virtual void onStart() override;
    virtual void onStop() override;
    virtual void onPause() override;
    virtual void onBlock(const Reason &why, const Trace &trace) override;
    virtual void onResume() override;
    virtual void onReset() override;
    virtual void onFinished(bool succ, const Reason &why, const Trace &trace) override;

  private:
    EventPublisher &pub_;
};

}
}

#endif //TBOX_FLOW_EVENT_ACTION_H_20221105
