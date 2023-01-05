#ifndef TBOX_EVENT_STAT_H
#define TBOX_EVENT_STAT_H

#include <cstdint>
#include <ostream>

namespace tbox {
namespace event {

struct Stat {
    uint64_t stat_time_us = 0;  //! 统计时间
    uint64_t time_cost_us = 0;  //! 占用时间
    uint32_t max_cost_us = 0;   //! 最大一次的消耗时长
    uint32_t event_count = 0;   //! 事件触发次数
};

}
}

std::ostream& operator<< (std::ostream &os, const tbox::event::Stat &stat);

#endif //TBOX_EVENT_STAT_H
