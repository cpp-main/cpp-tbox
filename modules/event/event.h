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
#ifndef TBOX_EVENT_ITEM_H_20170627
#define TBOX_EVENT_ITEM_H_20170627

#include <string>

namespace tbox {
namespace event {

class Loop;

class Event {
  public:
    Event(const std::string &what) : what_(what) { }

    enum class Mode {
        kPersist,
        kOneshot
    };

    virtual bool isEnabled() const = 0;
    virtual bool enable() = 0;
    virtual bool disable() = 0;

    virtual Loop* getLoop() const = 0;

    std::string what() const { return what_; }

  public:
    virtual ~Event() { }

  protected:
    std::string what_;
};

}
}

#endif //TBOX_EVENT_ITEM_H_20170627
