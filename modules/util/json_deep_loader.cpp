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
#include "json_deep_loader.h"
#include <fstream>

#include <tbox/base/json.hpp>
#include "json.h"
#include "string.h"
#include "fs.h"

namespace {
    const std::string kInclude = "__include__";
}

namespace tbox {
namespace util {
namespace json {

Json DeepLoader::load(const std::string &filename) {
    if (checkDuplicateInclude(filename))
        throw DuplicateIncludeError(filename);

    files_.push_back(filename);
    Json js = Load(filename);
    traverse(js);
    files_.pop_back();
    return js;
}

void DeepLoader::traverse(Json &js) {
    if (js.is_object()) {
        Json js_patch;
        for (auto &item : js.items()) {
            auto &js_value = item.value();
            if (item.key() == kInclude) {
                handleInclude(js_value, js_patch);
            } else {
                traverse(js_value);
            }
        }
        js.erase(kInclude);

        if (!js_patch.is_null())
            js.merge_patch(js_patch);

    } else if (js.is_array()) {
        for (auto &js_item : js) {
            traverse(js_item);
        }
    }
}

void DeepLoader::handleInclude(const Json &js_include, Json &js_parent) {
    if (js_include.is_string()) {
        includeByDescriptor(js_include.get<std::string>(), js_parent);
    } else if (js_include.is_array()) {
        for (auto &js_item : js_include) {
            if (js_item.is_string())
                includeByDescriptor(js_item.get<std::string>(), js_parent);
            else
                throw IncludeDescriptorTypeInvalid();
        }
    } else {
        throw IncludeDescriptorTypeInvalid();
    }
}

void DeepLoader::includeByDescriptor(const std::string &descriptor, Json &js) {
    std::vector<std::string> str_vec;
    string::Split(descriptor, "=>", str_vec);
    std::string filename = string::Strip(str_vec.at(0));

    std::string real_filename;
    if (filename.front() != '/') {
        auto dirname = fs::Dirname(files_.back());
        real_filename = dirname + '/' + filename;
    } else {
        real_filename = filename;
    }
 
    auto js_load = load(real_filename);

    if (str_vec.size() >= 2) {
        auto keyname = string::Strip(str_vec.at(1));
        js[keyname] = std::move(js_load);
    } else {
        js.merge_patch(std::move(js_load));
    }
}

bool DeepLoader::checkDuplicateInclude(const std::string &filename) const {
    return std::find(files_.begin(), files_.end(), filename) != files_.end();
}

Json LoadDeeply(const std::string &filename) {
    return DeepLoader().load(filename);
}

}
}
}
