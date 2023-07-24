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
#ifndef TBOX_BASE_JSON_H_20211227
#define TBOX_BASE_JSON_H_20211227

#include <nlohmann/json_fwd.hpp>

//! 如果找到不该文件，则从 github 下载：
//! https://github.com/nlohmann/json/blob/v3.10.4/include/nlohmann/json_fwd.hpp
//! 下载链接：https://raw.githubusercontent.com/nlohmann/json/v3.10.4/include/nlohmann/json_fwd.hpp
//! 放置到 /usr/local/include/nlohmann/ 目录下

namespace tbox {

using Json = nlohmann::json;
using OrderedJson = nlohmann::ordered_json;

}

#endif //TBOX_BASE_JSON_H_20211227
