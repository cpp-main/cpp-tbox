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
#ifndef TBOX_HTTP_SERVER_ROUTER_H_20220508
#define TBOX_HTTP_SERVER_ROUTER_H_20220508

#include "../middleware.h"
#include "../context.h"

namespace tbox {
namespace http {
namespace server {

/**
 * 路由中间件
 */
class RouterMiddleware : public Middleware {
  public:
    RouterMiddleware();
    ~RouterMiddleware();

  public:
    RouterMiddleware& get  (const std::string &path, RequestHandler &&handler);
    RouterMiddleware& post (const std::string &path, RequestHandler &&handler);
    RouterMiddleware& put  (const std::string &path, RequestHandler &&handler);
    RouterMiddleware& del  (const std::string &path, RequestHandler &&handler);
    RouterMiddleware& opt  (const std::string &path, RequestHandler &&handler);
    RouterMiddleware& head (const std::string &path, RequestHandler &&handler);
    RouterMiddleware& trace(const std::string &path, RequestHandler &&handler);

  public:
    virtual void handle(ContextSptr sp_ctx, const NextFunc &next) override;

  private:
    struct Data;
    Data *d_;
};

}
}
}

#endif //TBOX_HTTP_SERVER_ROUTER_H_20220508
