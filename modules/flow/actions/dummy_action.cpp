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
 * Copyright (c) 2025 Hevake and contributors, all rights reserved.
 *
 * This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
 * Use of this source code is governed by MIT license that can be found
 * in the LICENSE file in the root of the source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */
#include "dummy_action.h"

namespace tbox {
namespace flow {

void DummyAction::onStart() {
    Action::onStart();
    if (start_cb_)
        start_cb_();
}

void DummyAction::onStop() {
    if (stop_cb_)
        stop_cb_();
    Action::onStop();
}

void DummyAction::onPause() {
    if (pause_cb_)
        pause_cb_();
    Action::onPause();
}

void DummyAction::onResume() {
    Action::onResume();
    if (resume_cb_)
        resume_cb_();
}

void DummyAction::onReset() {
    if (reset_cb_)
        reset_cb_();
    Action::onReset();
}

}
}
