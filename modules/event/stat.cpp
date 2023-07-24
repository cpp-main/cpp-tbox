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
#include "stat.h"
#include <iomanip>

using namespace std;

ostream& operator<< (ostream &os, const tbox::event::Stat &stat)
{
    os << std::setprecision(3);

    os << "stat_time: " << stat.stat_time_us << " us" << endl;

    os << "loop_count: " << stat.loop_count << endl;
    os << "loop_acc_cost: " << stat.loop_acc_cost_us << " us" << endl;
    os << "loop_avg_cost: " << stat.loop_acc_cost_us * 1.0 / stat.loop_count << " us" << endl;
    os << "loop_peak_cost: " << stat.loop_peak_cost_us << " us" << endl;
    os << "loop_cpu: " << (stat.loop_acc_cost_us * 100.0 / stat.stat_time_us) << " %" << endl;

    os << "run_in_loop_peak_num: " << stat.run_in_loop_peak_num << endl;
    os << "run_next_peak_num: " << stat.run_next_peak_num << endl;

    return os;
}
