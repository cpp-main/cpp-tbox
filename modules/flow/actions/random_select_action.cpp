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
#include "random_select_action.h"

#include <random>
#include <tbox/base/log.h>
#include <tbox/base/json.hpp>

namespace tbox {
namespace flow {

using namespace std::placeholders;

RandomSelectAction::RandomSelectAction(event::Loop &loop)
  : SerialAssembleAction(loop, "RandomSelect")
{ }

RandomSelectAction::~RandomSelectAction() {
    for (auto action : children_)
        delete action;
}

void RandomSelectAction::toJson(Json &js) const {
    SerialAssembleAction::toJson(js);

    Json &js_children = js["children"];
    for (auto action : children_) {
        Json js_child;
        action->toJson(js_child);
        js_children.push_back(std::move(js_child));
    }
}

int RandomSelectAction::addChild(Action *child) {
    if (child == nullptr) {
        LogWarn("%d:%s[%s], add child %d:%s[%s] fail, child == nullptr",
                id(), type().c_str(), label().c_str());
        return false;
    }

    if (!child->setParent(this))
        return false;

    int index = children_.size();
    child->setFinishCallback(std::bind(&RandomSelectAction::onLastChildFinished, this, _1, _2, _3));
    child->setBlockCallback(std::bind(&RandomSelectAction::block, this, _1, _2));
    children_.push_back(child);

    return index;
}

bool RandomSelectAction::isReady() const {
    if (children_.empty()) {
        LogWarn("%d:%s[%s], no child, not ready");
        return false;
    }

    for (auto child : children_) {
        if (!child->isReady())
            return false;
    }
    return true;
}

void RandomSelectAction::onStart() {
    SerialAssembleAction::onStart();

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, children_.size() - 1);

    startThisAction(children_.at(dis(gen)));
}

void RandomSelectAction::onReset() {
    for (auto child : children_)
        child->reset();

    SerialAssembleAction::onReset();
}

}
}
