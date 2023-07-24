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
#ifndef TBOX_HTTP_CLIENT_H_20220504
#define TBOX_HTTP_CLIENT_H_20220504

#include <tbox/event/loop.h>
#include <tbox/network/sockaddr.h>

#include "../request.h"
#include "../respond.h"

namespace tbox {
namespace http {
namespace client {

class Client {
  public:
    explicit Client(event::Loop *wp_loop);
    virtual ~Client();

  public:
    //! 初始化，设置目标服务器
    bool initialize(const network::SockAddr &server_addr);

    //! 收到回复时的回调
    using RespondCallback = std::function<void(const Respond &res)>;

    /**
     * \brief   发送请求
     * \param   req     请求数据
     * \param   cb      回复的回调
     */
    void request(const Request &req, const RespondCallback &cb);

    //! 清理，与initialize()是逆操作
    void cleanup();

  private:
    class Impl;
    Impl *impl_;
};

}
}
}

#endif //TBOX_HTTP_CLIENT_H_20220504
