#ifndef TBOX_TIMER_WORKDAY_CALENDAR_H_20221024
#define TBOX_TIMER_WORKDAY_CALENDAR_H_20221024

#include <map>
#include <vector>

namespace tbox {
namespace timer {

class WorkdayTimer;

/**
 * \brief 工作日的日历
 *
 * 它用于向 WorkdayTimer 提供指定日期是否为工作日查询的服务
 */
class WorkdayCalendar {
  public:
    /**
     * \brief 更新特殊的节假日与补班日期列表
     *
     * \param special_days  特殊的日期列表，key为日期，value为是否为工作日
     */
    void updateSpecialDays(const std::map<int, bool> &special_days);

    /**
     * \brief 更新一周默认的工作日掩码
     *
     * 通常周一到周五是工作日，周六与周日是非工作日
     * 通常不用设置。但是有地区存在差异，比如施行一周四天工作的地区。这种情况就得设置一下。
     */
    void updateWeekMask(uint8_t week_mask);

    void subscribe(WorkdayTimer *timer);
    void unsubscribe(WorkdayTimer *timer);

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
    uint8_t week_mask_ = 0b01111110;    //!< 默认周一到周五是工作日
    std::map<int, bool> special_days_;  //!< 特殊的放假与补班日期
    std::vector<WorkdayTimer*> watch_timers_;
};

}
}

#endif //TBOX_TIMER_WORKDAY_CALENDAR_H_20221024
