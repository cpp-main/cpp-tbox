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
#ifndef TBOX_HTTP_SERVER_TYPES_H_20220503
#define TBOX_HTTP_SERVER_TYPES_H_20220503

#include <memory>
#include <functional>

namespace tbox {
namespace http {
namespace server {

class Context;
using ContextSptr = std::shared_ptr<Context>;
using NextFunc = std::function<void()>;
using RequestCallback = std::function<void(ContextSptr, const NextFunc &)>;

}
}
}

#endif //TBOX_HTTP_SERVER_TYPES_H_20220503
