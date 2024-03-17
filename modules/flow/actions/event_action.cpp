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
#include "event_action.h"

namespace tbox {
namespace flow {

EventAction::EventAction(event::Loop &loop, const std::string &type, EventPublisher &pub)
  : Action(loop, type)
  , pub_(pub)
{ }

EventAction::~EventAction() {
    if (state() == State::kRunning)
        pub_.unsubscribe(this);
}

void EventAction::onStart() {
    Action::onStart();
    pub_.subscribe(this);
}

void EventAction::onStop() {
    pub_.unsubscribe(this);
    Action::onStop();
}

void EventAction::onPause() {
    pub_.unsubscribe(this);
    Action::onPause();
}

void EventAction::onBlock(const Reason &why, const Trace &trace) {
    pub_.unsubscribe(this);
    Action::onBlock(why, trace);
}

void EventAction::onResume() {
    Action::onResume();
    pub_.subscribe(this);
}

void EventAction::onReset() {
    pub_.unsubscribe(this);
    Action::onReset();
}

void EventAction::onFinished(bool is_succ, const Reason &why, const Trace &trace) {
    pub_.unsubscribe(this);
    Action::onFinished(is_succ, why, trace);
}

}
}
