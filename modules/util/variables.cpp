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
#include "variables.h"

#include <tbox/base/json.hpp>

namespace tbox {
namespace util {

Variables::Variables() { }

Variables::~Variables() {
    CHECK_DELETE_RESET_OBJ(var_map_);
    parent_ = nullptr;
}

IMPL_COPY_FUNC(Variables)

IMPL_MOVE_RESET_FUNC(Variables)

void Variables::swap(Variables &other) {
    std::swap(var_map_, other.var_map_);
    std::swap(parent_, other.parent_);
}

void Variables::copy(const Variables &other) {
    parent_ = other.parent_;
    if (other.var_map_ != nullptr) {
        if (var_map_ == nullptr)
            var_map_ = new VariableMap;
        *var_map_ = *other.var_map_;
    }
}

bool Variables::define(const std::string &name, const Json &js_init_value) {
    if (var_map_ == nullptr)
        var_map_ = new VariableMap;

    auto result = var_map_->emplace(name, js_init_value);
    return result.second;
}

bool Variables::undefine(const std::string &name) {
    if (var_map_ == nullptr)
        return false;

    auto iter = var_map_->find(name);
    if (iter == var_map_->end())
        return false;

    var_map_->erase(iter);

    if (var_map_->empty())
        CHECK_DELETE_RESET_OBJ(var_map_);

    return true;
}

bool Variables::has(const std::string &name, bool local_only) const {
    if (var_map_ != nullptr) {
        auto iter = var_map_->find(name);
        if (iter != var_map_->end())
            return true;
    }

    //! 本节点找不到，就找父节点
    if (!local_only && parent_ != nullptr)
        return parent_->has(name);

    return false;
}

bool Variables::get(const std::string &name, Json &js_out_value, bool local_only) const {
    if (var_map_ != nullptr) {
        auto iter = var_map_->find(name);
        if (iter != var_map_->end()) {
            js_out_value = iter->second;
            return true;
        }
    }

    //! 本节点找不到，就找父节点
    if (!local_only && parent_ != nullptr)
        return parent_->get(name, js_out_value);

    return false;
}

bool Variables::set(const std::string &name, const Json &js_new_value, bool local_only) {
    if (var_map_ != nullptr) {
        auto iter = var_map_->find(name);
        if (iter != var_map_->end()) {
            iter->second = js_new_value;
            return true;
        }
    }

    //! 本节点找不到，就找父节点
    if (!local_only && parent_ != nullptr)
        return parent_->set(name, js_new_value);

    return false;
}

void Variables::toJson(Json &js) const {
    if (var_map_ != nullptr)
        js = *var_map_;
    else
        js = Json::object();
}

}
}
