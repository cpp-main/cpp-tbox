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
#include "router.h"
#include <map>

namespace tbox {
namespace http {
namespace server {

struct Router::Data {
    std::map<std::string, RequestCallback> cbs_;
};

Router::Router() :
    d_(new Data)
{ }

Router::~Router()
{
    delete d_;
}

void Router::handle(ContextSptr sp_ctx, const NextFunc &next)
{
    std::string prefix;
    switch (sp_ctx->req().method) {
        case Method::kGet:      prefix = "get:";   break;
        case Method::kPut:      prefix = "put:";   break;
        case Method::kPost:     prefix = "post:";  break;
        case Method::kDelete:   prefix = "del:";   break;
        default:;
    }

    auto iter = d_->cbs_.find(prefix + sp_ctx->req().url.path);
    if (iter != d_->cbs_.end()) {
        if (iter->second) {
            iter->second(sp_ctx, next);
            return;
        }
    }
    next();
}

Router& Router::get(const std::string &path, const RequestCallback &cb)
{
    d_->cbs_[std::string("get:") + path] = cb;
    return *this;
}

Router& Router::post(const std::string &path, const RequestCallback &cb)
{
    d_->cbs_[std::string("post:") + path] = cb;
    return *this;
}

Router& Router::put(const std::string &path, const RequestCallback &cb)
{
    d_->cbs_[std::string("put:") + path] = cb;
    return *this;
}

Router& Router::del(const std::string &path, const RequestCallback &cb)
{
    d_->cbs_[std::string("del:") + path] = cb;
    return *this;
}

}
}
}
