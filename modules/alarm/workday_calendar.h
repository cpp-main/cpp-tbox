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
#ifndef TBOX_ALARM_WORKDAY_CALENDAR_H_20221024
#define TBOX_ALARM_WORKDAY_CALENDAR_H_20221024

#include <cstdint>
#include <map>
#include <vector>

namespace tbox {
namespace alarm {

class Alarm;

/**
 * \brief 工作日的日历
 *
 * 它用于向 WorkdayAlarm 提供指定日期是否为工作日查询的功能
 */
class WorkdayCalendar {
  public:
    /**
     * \brief 更新特殊的节假日与补班日期列表
     *
     * \param special_days  特殊的日期表
     *                      key：从1970-1-1至所指定日期的天数
     *                      value：是否为工作日，true表示工作日，false表示节假日
     */
    void updateSpecialDays(const std::map<int, bool> &special_days);

    /**
     * \brief 修改一周默认的工作日
     *
     * 通常周一到周五是工作日，周六与周日是非工作日，不用去设置。
     * 但是有地区存在差异。比如某些地区福利极好，实施一周四天工作制，则需要通过该方法修改。
     */
    void updateWeekMask(uint8_t week_mask);

    void subscribe(Alarm *alarm);
    void unsubscribe(Alarm *alarm);

    /**
     * \brief 查询指定日期是否为工作日
     *
     * \param day_index   从1970-1-1至指定日期的天数
     *
     * \return  true      是工作日
     *          false     不是工作日
     */
    bool isWorkay(int day_index) const;

  private:
    uint8_t week_mask_ = 0b00111110;    //!< 默认周一到周五是工作日
    std::map<int, bool> special_days_;  //!< 特殊的放假与补班日期
    std::vector<Alarm*> watch_alarms_;
};

}
}

#endif //TBOX_ALARM_WORKDAY_CALENDAR_H_20221024
