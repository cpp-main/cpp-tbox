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
#include "module.h"

#include <algorithm>

#include <tbox/base/log.h>
#include <tbox/base/json.hpp>

namespace tbox {
namespace main {

Module::Module(const std::string &name, Context &ctx) :
    name_(name), ctx_(ctx)
{ }

Module::~Module()
{
    cleanup();

    for (const auto &item : children_)
        delete item.module_ptr;
    children_.clear();
}

bool Module::add(Module *child, bool required)
{
    if (state_ != State::kNone) {
        LogWarn("module %s's state is not State::kNone", name_.c_str());
        return false;
    }

    if (child == nullptr) {
        LogWarn("child == nullptr");
        return false;
    }

    //! 防止child被重复添加到多个Module上
    if (child->parent_ != nullptr) {
        LogWarn("module %s can't add %s, it has parent", name_.c_str(), child->name_.c_str());
        return false;
    }

    //! 检查名称有没有重复的
    auto iter = std::find_if(children_.begin(), children_.end(),
        [child] (const ModuleItem &item) {
            return item.module_ptr->name() == child->name();
        }
    );
    if (iter != children_.end()) {
        LogWarn("module %s can't add %s, name dupicated.", name_.c_str(), child->name().c_str());
        return false;
    }

    children_.emplace_back(ModuleItem{ child, required });
    child->vars_.setParent(&vars_);
    child->parent_ = this;

    return true;
}

bool Module::addAs(Module *child, const std::string &name, bool required)
{
    auto tmp = child->name_;
    child->name_ = name;

    if (!add(child, required)) {
        //! 如果失败了，还原之前的名称
        child->name_ = tmp;
        return false;
    }

    return true;
}

void Module::fillDefaultConfig(Json &js_parent)
{
    Json &js_this = name_.empty() ? js_parent : js_parent[name_];

    onFillDefaultConfig(js_this);

    for (const auto &item : children_)
        item.module_ptr->fillDefaultConfig(js_this);
}

bool Module::initialize(const Json &js_parent)
{
    if (state_ != State::kNone) {
        LogWarn("module %s's state is not State::kNone", name_.c_str());
        return false;
    }

    if (!name_.empty() && !js_parent.contains(name_)) {
        LogWarn("missing `%s' field in config", name_.c_str());
        return false;
    }

    const Json &js_this = name_.empty() ? js_parent : js_parent[name_];

    if (!onInit(js_this)) {
        LogWarn("module %s init fail.", name_.c_str());
        return false;
    }

    for (const auto &item : children_) {
        if (!item.module_ptr->initialize(js_this) && item.required) {
            LogErr("required module `%s' initialize() fail", item.module_ptr->name().c_str());
            return false;
        }
    }

    state_ = State::kInited;
    return true;
}

bool Module::start()
{
    if (state_ != State::kInited) {
        LogWarn("module %s's state is not State::kInited", name_.c_str());
        return false;
    }

    if (!onStart()) {
        LogWarn("module %s start fail.", name_.c_str());
        return false;
    }

    for (const auto &item : children_) {
        if (!item.module_ptr->start() && item.required) {
            LogErr("required module `%s' start() fail", item.module_ptr->name().c_str());
            return false;
        }
    }

    state_ = State::kRunning;
    return true;
}

void Module::stop()
{
    if (state_ != State::kRunning)
        return;

    for (auto iter = children_.rbegin(); iter != children_.rend(); ++iter)
        iter->module_ptr->stop();

    onStop();

    state_ = State::kInited;
}

void Module::cleanup()
{
    if (state_ == State::kNone)
        return;

    stop();

    for (auto iter = children_.rbegin(); iter != children_.rend(); ++iter)
        iter->module_ptr->cleanup();

    onCleanup();

    state_ = State::kNone;
}

void Module::toJson(Json &js) const
{
    if (!vars_.empty())
        vars_.toJson(js["vars"]);

    if (!children_.empty()) {
        Json &js_children = js["children"];
        for (auto &item : children_) {
            Json &js_child = js_children[item.module_ptr->name()];
            js_child["required"] = item.required;
            item.module_ptr->toJson(js_child);
        }
    }
}

}
}
