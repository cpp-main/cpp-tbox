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
#include "form_data.h"

namespace tbox {
namespace http {
namespace server {

void FormData::addItem(const FormItem& item) {
    items_.push_back(item);
    size_t index = items_.size() - 1;
    name_index_[item.name].push_back(index);
}

const std::vector<FormItem>& FormData::items() const {
    return items_;
}

std::vector<FormItem> FormData::getItems(const std::string& name) const {
    std::vector<FormItem> result;
    auto it = name_index_.find(name);
    if (it != name_index_.end()) {
        for (auto index : it->second) {
            result.push_back(items_[index]);
        }
    }
    return result;
}

bool FormData::getItem(const std::string& name, FormItem& item) const {
    auto it = name_index_.find(name);
    if (it != name_index_.end() && !it->second.empty()) {
        item = items_[it->second.front()];
        return true;
    }
    return false;
}

bool FormData::getField(const std::string& name, std::string& value) const {
    FormItem item;
    if (getItem(name, item) && item.type == FormItem::Type::kField) {
        value = item.value;
        return true;
    }
    return false;
}

bool FormData::getFile(const std::string& name, std::string& filename, std::string& content) const {
    FormItem item;
    if (getItem(name, item) && item.type == FormItem::Type::kFile) {
        filename = item.filename;
        content = item.value;
        return true;
    }
    return false;
}

bool FormData::contains(const std::string& name) const {
    return name_index_.find(name) != name_index_.end();
}

void FormData::clear() {
    items_.clear();
    name_index_.clear();
}

}
}
}
