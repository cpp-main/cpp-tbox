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
#ifndef TBOX_VARIABLES_H_20250201
#define TBOX_VARIABLES_H_20250201

#include <map>

#include <tbox/base/defines.h>
#include <tbox/base/json_fwd.h>

#include "json.h"

namespace tbox {
namespace util {

//! 变量对象
class Variables {
  public:
    Variables();
    ~Variables();

    DECLARE_COPY_FUNC(Variables);
    DECLARE_MOVE_RESET_FUNC(Variables);

  public:
    void setParent(Variables *parent) { parent_ = parent; }

    /**
     * 定义变量，并指定初始值
     *
     * \param name          变量名
     * \param js_init_value 变量初始值
     *
     * \return true     成功
     * \return false    失败，已存在
     */
    bool define(const std::string &name, const Json &js_init_value);

    /**
     * 删除变量
     */
    bool undefine(const std::string &name);

    /**
     * 检查变量是否存在
     *
     * \param name          变量名
     * \param local_only    是否仅在本地寻找
     *
     * \return true     存在
     * \return false    不存在
     */
    bool has(const std::string &name, bool local_only = false) const;

    /**
     * 获取变量
     *
     * \param name          变量名
     * \param js_out_value  获取的变量值
     * \param local_only    是否仅在本地寻找
     *
     * \return true     成功
     * \return false    失败，变量不存在
     */
    bool get(const std::string &name, Json &js_out_value, bool local_only = false) const;

    //! 获取变量的模板
    template <typename T>
    bool get(const std::string &name, T &out_value, bool local_only = false) const {
        Json js;
        if (get(name, js, local_only))
            return json::Get(js, out_value);
        return false;
    }

    /**
     * 更新变量
     *
     * \param name          变量名
     * \param js_new_value  新值
     * \param local_only    是否仅在本地寻找
     *
     * \return true     成功
     * \return false    失败，变量不存在
     */
    bool set(const std::string &name, const Json &js_new_value, bool local_only = false);

    //! 检查是否没有变量
    inline bool empty() const { return var_map_ == nullptr; }

  public:
    void toJson(Json &js) const;        //! 导出为Json对象

    void swap(Variables &other);        //! 交换
    void copy(const Variables &other);  //! 复制

  private:
    using VariableMap = std::map<std::string, Json>;

    Variables *parent_ = nullptr;
    VariableMap *var_map_ = nullptr;
};

}
}

#endif //TBOX_VARIABLES_H_20250201
