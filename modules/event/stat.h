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
#ifndef TBOX_EVENT_STAT_H
#define TBOX_EVENT_STAT_H

#include <cstdint>
#include <ostream>

namespace tbox {
namespace event {

struct Stat {
    uint64_t stat_time_us = 0;        //! 统计时长

    uint64_t loop_count = 0;          //! 循环次数
    uint64_t loop_acc_cost_us = 0;    //! 循环执行累积时长
    uint64_t loop_peak_cost_us = 0;   //! 循环执行时长峰值

    size_t   run_in_loop_peak_num = 0;  //!< 等待任务数峰值
    size_t   run_next_peak_num = 0;   //!< 等待任务数峰值
};

}
}

std::ostream& operator<< (std::ostream &os, const tbox::event::Stat &stat);

#endif //TBOX_EVENT_STAT_H
