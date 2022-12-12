#include "workday_calendar.h"
#include <gtest/gtest.h>

namespace tbox {
namespace alarm {

//! 模拟 2022年国庆的放假
TEST(WorkdayCalendar, Base) {
  WorkdayCalendar cal;

  std::map<int, bool> days;
  days[19268] = false;  //! 2022-10-03  放假
  days[19269] = false;  //! 2022-10-04  放假
  days[19270] = false;  //! 2022-10-05  放假
  days[19271] = false;  //! 2022-10-06  放假
  days[19272] = false;  //! 2022-10-07  放假
  days[19273] = true;   //! 2022-10-08  补班
  days[19274] = true;   //! 2022-10-09  补班

  cal.updateSpecialDays(days);

  EXPECT_TRUE (cal.isWorkay(19265));  //! 2022-09-30  正常上班
  EXPECT_FALSE(cal.isWorkay(19266));  //! 2022-10-01  周六放假
  EXPECT_FALSE(cal.isWorkay(19267));  //! 2022-10-02  周日放假
  EXPECT_FALSE(cal.isWorkay(19268));  //! 2022-10-03  放假
  EXPECT_FALSE(cal.isWorkay(19269));  //! 2022-10-04  放假
  EXPECT_FALSE(cal.isWorkay(19270));  //! 2022-10-05  放假
  EXPECT_FALSE(cal.isWorkay(19271));  //! 2022-10-06  放假
  EXPECT_FALSE(cal.isWorkay(19272));  //! 2022-10-07  放假
  EXPECT_TRUE (cal.isWorkay(19273));  //! 2022-10-08  补班
  EXPECT_TRUE (cal.isWorkay(19274));  //! 2022-10-09  补班
}

}
}
