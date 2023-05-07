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
