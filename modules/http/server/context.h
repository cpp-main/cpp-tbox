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
#ifndef TBOX_HTTP_CONTEXT_H_20220502
#define TBOX_HTTP_CONTEXT_H_20220502

#include <tbox/base/defines.h>
#include <tbox/base/cabinet_token.h>

#include "../common.h"
#include "../request.h"
#include "../respond.h"

namespace tbox {
namespace http {
namespace server {

class Server;

/**
 * Http请求上下文
 */
class Context {
  public:
    Context(Server *wp_server, const cabinet::Token &ct,
            int req_index, Request *req);
    ~Context();

    NONCOPYABLE(Context);

  public:
    Request& req() const;
    Respond& res() const;   //! 注意: 在 done() 之后就不可以再使用该函数

  private:
    struct Data;
    Data *d_;
};

}
}
}

#endif //TBOX_HTTP_CONTEXT_H_20220502
