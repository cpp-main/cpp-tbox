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
#ifndef TBOX_FLOW_EVENT_H_20221022
#define TBOX_FLOW_EVENT_H_20221022

namespace tbox {
namespace flow {

struct Event {
  using ID = int;

  ID id = 0;
  const void *extra = nullptr;

  Event() { }

  template <typename ET>
    Event(ET e) : id(static_cast<ID>(e)) { }

  template <typename ET, typename DT>
    Event(ET e, DT *p) : id(static_cast<ID>(e)), extra(p) { }
};

}
}

#endif //TBOX_FLOW_EVENT_H_20221022
