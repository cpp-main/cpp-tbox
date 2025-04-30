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
#ifndef TBOX_HTTP_SERVER_FORM_DATA_H_20250419
#define TBOX_HTTP_SERVER_FORM_DATA_H_20250419

#include <string>
#include <map>
#include <vector>

namespace tbox {
namespace http {
namespace server {

/**
 * 表单项类型，可以是普通的文本字段或文件
 */
struct FormItem {
    enum class Type {
        kField,  //! 普通字段
        kFile    //! 文件
    };

    Type type = Type::kField;
    std::string name;                 //! 字段名称
    std::string value;                //! 字段值（对于普通字段）
    std::string filename;             //! 文件名（对于文件）
    std::string content_type;         //! 内容类型
    std::map<std::string, std::string> headers;  //! 其他头部信息
};

/**
 * 表单数据类，存储解析后的表单数据
 */
class FormData {
  public:
    FormData() = default;
    ~FormData() = default;

    /**
     * 添加一个表单项
     */
    void addItem(const FormItem& item);

    /**
     * 获取所有表单项
     */
    const std::vector<FormItem>& items() const;

    /**
     * 获取具有指定名称的所有表单项
     */
    std::vector<FormItem> getItems(const std::string& name) const;

    /**
     * 获取指定名称的第一个表单项
     */
    bool getItem(const std::string& name, FormItem& item) const;

    /**
     * 获取指定名称的第一个表单项的值（如果是字段）
     */
    bool getField(const std::string& name, std::string& value) const;

    /**
     * 获取指定名称的第一个文件（如果是文件）
     */
    bool getFile(const std::string& name, std::string& filename, std::string& content) const;

    /**
     * 检查是否包含指定名称的表单项
     */
    bool contains(const std::string& name) const;

    /**
     * 清空表单数据
     */
    void clear();

  private:
    std::vector<FormItem> items_;
    std::map<std::string, std::vector<size_t>> name_index_;  //! 名称到索引的映射
};

}
}
}

#endif // TBOX_HTTP_SERVER_FORM_DATA_H_20250419
