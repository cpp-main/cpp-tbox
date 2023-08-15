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
 * Copyright (c) 2023 Hevake and contributors, all rights reserved.
 *
 * This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
 * Use of this source code is governed by MIT license that can be found
 * in the LICENSE file in the root of the source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */
#ifndef TBOX_COROUTINE_MUTEX_HPP_20180527
#define TBOX_COROUTINE_MUTEX_HPP_20180527

#include <queue>
#include "scheduler.h"

namespace tbox {
namespace coroutine {

//! 信号量
class Mutex {
  public:
    class Locker {
      public:
        Locker(Mutex &m) : m_(m) { m_.lock(); }
        ~Locker() { m_.unlock(); }
      private:
        Mutex &m_;
    };

  public:
    Mutex(Scheduler &sch) : sch_(sch) { }

    //! 请求资源，注意：只能是协程调用
    //! 不建议直接使用，优先使用 Mutex::Locker 替代
    bool lock() {
        if (!hold_token_.isNull()) {      //! 如果没有资源，则等待
            if (hold_token_.equal(sch_.getToken())) //! 如果就是自己占用的，就直接返回
                return true;

            wait_tokens_.push(sch_.getToken());
            do {
                sch_.wait();
                if (sch_.isCanceled())
                    return false;
            } while (!hold_token_.isNull());
        }

        hold_token_ = sch_.getToken();
        return true;
    }

    //! 释放资源，注意：也只能是协程调用
    //! 不建议直接使用，优先使用 Mutex::Locker 替代
    void unlock() {
        if (!sch_.getToken().equal(hold_token_))
            return;

        hold_token_.reset();

        if (!wait_tokens_.empty()) {
            auto t = wait_tokens_.front();
            wait_tokens_.pop();
            sch_.resume(t);
        }
    }

  private:
    Scheduler &sch_;

    RoutineToken hold_token_;
    std::queue<RoutineToken> wait_tokens_;
};

}
}

#endif //TBOX_COROUTINE_MUTEX_HPP_20180527
