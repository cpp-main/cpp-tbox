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
#ifndef TBOX_UTIL_JSON_DEEP_LOADER_H_20221224
#define TBOX_UTIL_JSON_DEEP_LOADER_H_20221224

/**
 * 本文定义了DeepLoader，是一种JSON文件加载器。
 *
 * 它与util/json.h中实现的Load()不同的是：它会检查JSON数据中有没有"__include__"关键对象。
 * 如果存在，它会进一步去解析其中的描述，并将描述中所指向的其它JSON文件也加载进来，合并到本
 * 对象中。
 *
 * "__include__" 对象有两种格式："xxxxx" 与 ["xxxxx", "yyyyy", ...]
 * 前者是一个导入字串，表示包含导入单个JSON文件。后者是一个列表，表示要导入多个JSON文件。
 *
 * 导入字串格式："<JSON文件名>[=>结点ID]"
 * 如："sub.json", "sub.json => abc"，前者表示将 sub.json 中的内容导入到进当前结点，后者表示将
 * sub.json 中的内容导入到 "abc" 这个结点上。
 *
 * 以下为要导入的JSON文件：
 * |- main.json
 * |- common.json
 * |- sub3.json
 * `- sub
 *    |- sub1.json
 *    `- sub2.json
 *
 * 内容分别为：
 * main.json
 * {
 *   "main.a": 1,
 *   "__include__":["sub/sub1.json => sub1", "common.json"],
 * }
 *
 * sub/sub1.json
 * {
 *   "sub1.a": 1,
 *   "__include__":"sub2.json => sub2"
 * }
 *
 * common.json
 * {
 *   "common.a": 2
 * }
 *
 * sub/sub2.json
 * [1, 2, 3, { "__include__": "../sub3.json" }]
 *
 * sub3.json
 * {
 *   "sub3.a": 4
 * }
 *
 * 加载后结果为：
 * {
 *   "common.a": 2,
 *   "main.a" : 1,
 *   "sub1": {
 *      "sub1.a": 1,
 *      "sub2": [1, 2, 3, { "sub3.a": 4 }]
 *   },
 * }
 *
 */

#include <stdexcept>
#include <tbox/base/json_fwd.h>

namespace tbox {
namespace util {
namespace json {

//! include描述类型不合法，不是string
struct IncludeDescriptorTypeInvalid: public std::runtime_error {
    explicit IncludeDescriptorTypeInvalid() :
        std::runtime_error("include descriptor type error, it should be string") { }
};
//! 重复include同一个文件
struct DuplicateIncludeError : public std::runtime_error {
    explicit DuplicateIncludeError(const std::string &include_file) :
        std::runtime_error("duplicate include file:" + include_file) { }
};

class DeepLoader {
 public:
  Json load(const std::string &filename);

 protected:
  void traverse(Json &js);
  void handleInclude(const Json &js_include, Json &js_parent);
  void includeByDescriptor(const std::string &descriptor, Json &js);
  bool checkDuplicateInclude(const std::string &filename) const;

 private:
  std::vector<std::string> files_;
};

/// 加载JSON文件，并支持深度加载
/**
 * \param filename  JSON文件名
 * \return Json     解析所得的Json对象
 *
 * \throw OpenFileError
 *        ParseJsonFileError
 *        IncludeDescriptorTypeInvalid
 *        DuplicateIncludeError
 */
Json LoadDeeply(const std::string &filename);

}
}
}

#endif //TBOX_UTIL_JSON_DEEP_LOADER_H_20221224
