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
#ifndef TBOX_HTTP_SERVER_FORM_DATA_MIDDLEWARE_H_20250419
#define TBOX_HTTP_SERVER_FORM_DATA_MIDDLEWARE_H_20250419

#include <tbox/base/defines.h>

#include "../middleware.h"
#include "../context.h"
#include "form_data.h"

namespace tbox {
namespace http {
namespace server {

/**
 * 表单中间件，用于处理：
 * - multipart/form-data
 * - application/x-www-form-urlencoded
 */
class FormDataMiddleware : public Middleware {
  public:
    explicit FormDataMiddleware();
    ~FormDataMiddleware();

    NONCOPYABLE(FormDataMiddleware);

    //! 处理表单数据的回调函数类型
    using FormHandler = std::function<void(ContextSptr, const FormData&, const NextFunc&)>;

    /**
     * 注册POST请求处理器
     *
     * @param path 请求路径
     * @param handler 处理函数
     */
    void post(const std::string& path, FormHandler&& handler);

    /**
     * 注册GET请求处理器
     *
     * @param path 请求路径
     * @param handler 处理函数
     */
    void get(const std::string& path, FormHandler&& handler);

    /**
     * 注册特定Method的请求处理器
     *
     * @param method HTTP方法
     * @param path 请求路径
     * @param handler 处理函数
     */
    void registerHandler(Method method, const std::string& path, FormHandler&& handler);

  protected:
    /**
     * 中间件处理函数，解析multipart/form-data数据
     */
    virtual void handle(ContextSptr sp_ctx, const NextFunc& next) override;

  private:

    struct Data;
    Data *d_;
};

}
}
}

#endif // TBOX_HTTP_SERVER_FORM_DATA_MIDDLEWARE_H_20250419
