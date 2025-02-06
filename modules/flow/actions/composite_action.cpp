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
#include "composite_action.h"
#include <tbox/base/defines.h>
#include <tbox/base/assert.h>
#include <tbox/base/json.hpp>

namespace tbox {
namespace flow {

using namespace std::placeholders;

CompositeAction::~CompositeAction() {
    CHECK_DELETE_RESET_OBJ(child_);
}

bool CompositeAction::setChild(Action *child) {
    if (child == nullptr) {
        LogWarn("%d:%s[%s], add child %d:%s[%s] fail, child == nullptr",
                id(), type().c_str(), label().c_str());
        return false;
    }

    if (!child->setParent(this))
        return false;

    child->setFinishCallback(std::bind(&CompositeAction::onLastChildFinished, this, _1, _2, _3));
    child->setBlockCallback(std::bind(&CompositeAction::block, this, _1, _2));

    CHECK_DELETE_RESET_OBJ(child_);
    child_ = child;

    return true;
}

void CompositeAction::toJson(Json &js) const {
    SerialAssembleAction::toJson(js);

    TBOX_ASSERT(child_ != nullptr);
    child_->toJson(js["child"]);
}

bool CompositeAction::isReady() const {
    if (child_ != nullptr)
        return child_->isReady();

    LogWarn("%d:%s[%s] child not set", id(), type().c_str(), label().c_str());
    return false;
}

void CompositeAction::onStart() {
    SerialAssembleAction::onStart();

    TBOX_ASSERT(child_ != nullptr);
    startThisAction(child_);
}

void CompositeAction::onReset() {
    TBOX_ASSERT(child_ != nullptr);
    child_->reset();

    SerialAssembleAction::onReset();
}

void CompositeAction::onFinished(bool is_succ, const Reason &why, const Trace &trace) {
    //! 有可能不是child_自然结束产生的finish
    stopCurrAction();

    SerialAssembleAction::onFinished(is_succ, why, trace);
}

}
}
