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
#include "router_middleware.h"
#include <map>
#include "route_key.h"

namespace tbox {
namespace http {
namespace server {

struct RouterMiddleware::Data {
    std::map<RouteKey, RequestHandler> route_handles;
};

RouterMiddleware::RouterMiddleware() : d_(new Data)
{ }

RouterMiddleware::~RouterMiddleware()
{
    delete d_;
}

void RouterMiddleware::handle(ContextSptr sp_ctx, const NextFunc &next)
{
    RouteKey key{sp_ctx->req().method, sp_ctx->req().url.path};

    auto iter = d_->route_handles.find(key);
    if (iter != d_->route_handles.end()) {
        if (iter->second) {
            iter->second(sp_ctx, next);
            return;
        }
    }
    next();
}

RouterMiddleware& RouterMiddleware::get(const std::string &path, RequestHandler &&handler)
{
    d_->route_handles[RouteKey{Method::kGet, path}] = std::move(handler);
    return *this;
}

RouterMiddleware& RouterMiddleware::post(const std::string &path, RequestHandler &&handler)
{
    d_->route_handles[RouteKey{Method::kPost, path}] = std::move(handler);
    return *this;
}

RouterMiddleware& RouterMiddleware::put(const std::string &path, RequestHandler &&handler)
{
    d_->route_handles[RouteKey{Method::kPut, path}] = std::move(handler);
    return *this;
}

RouterMiddleware& RouterMiddleware::del(const std::string &path, RequestHandler &&handler)
{
    d_->route_handles[RouteKey{Method::kDelete, path}] = std::move(handler);
    return *this;
}

RouterMiddleware& RouterMiddleware::opt(const std::string &path, RequestHandler &&handler)
{
    d_->route_handles[RouteKey{Method::kOptions, path}] = std::move(handler);
    return *this;
}

RouterMiddleware& RouterMiddleware::head(const std::string &path, RequestHandler &&handler)
{
    d_->route_handles[RouteKey{Method::kHead, path}] = std::move(handler);
    return *this;
}

RouterMiddleware& RouterMiddleware::trace(const std::string &path, RequestHandler &&handler)
{
    d_->route_handles[RouteKey{Method::kTrace, path}] = std::move(handler);
    return *this;
}

}
}
}
