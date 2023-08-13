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
#ifndef TBOX_COROUTINE_SEMAPHORE_HPP_20180527
#define TBOX_COROUTINE_SEMAPHORE_HPP_20180527

#include <queue>
#include "scheduler.h"

namespace tbox {
namespace coroutine {

//! 信号量
class Semaphore {
  public:
    Semaphore(Scheduler &sch, int init_num) : sch_(sch), count_(init_num) { }

    //! 请求资源，注意：只能是协程调用
    bool acquire () {
        if (count_ == 0) {      //! 如果没有资源，则等待
            token_.push(sch_.getToken());
            do {
                sch_.wait();
                if (sch_.isCanceled())
                    return false;
            } while (count_ == 0);
        }

        --count_;
        return true;
    }

    //! 释放资源
    void release() {
        if (count_ == 0 && !token_.empty()) {
            auto t = token_.front();
            token_.pop();
            sch_.resume(t);
        }
        ++count_;
    }

    inline bool count() const { return count_; }

  private:
    Scheduler &sch_;

    int count_;
    std::queue<RoutineToken> token_;
};

}
}

#endif //TBOX_COROUTINE_SEMAPHORE_HPP_20180527
