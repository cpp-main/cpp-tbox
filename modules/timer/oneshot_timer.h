#ifndef TBOX_TIMER_ONESHOT_TIMER_H_20221023
#define TBOX_TIMER_ONESHOT_TIMER_H_20221023

#include "timer.h"

namespace tbox {
namespace timer {

/*
 * @brief The oneshot timer.
 *
 * OneshotTimer allow the user to make plans which execute at today or tomorrow
 * If the specified time is later than the local time, the system will execute today.
 * Otherwise it will be executed tomorrow.
 *
 * code example:
 *
 * Loop *loop = Loop::New();
 *
 * OneshotTimer tmr(loop);
 * tmr.initialize(30600); // at 08:30
 * tmr.setCallback([] { std::cout << "time is up" << endl;});
 * tmr.enable();
 *
 * loop->runLoop();
 */
class OneshotTimer : public Timer
{
  public:
    using Timer::Timer;

    /**
     * \brief 初始化
     *
     * \param seconds_of_day  从本地00:00起到触发时间的秒数
     *                        如：08:30 = 8 * 3600 + 30 * 60 = 30600
     */
    bool initialize(int seconds_of_day);

  protected:
    virtual int calculateWaitSeconds(uint32_t curr_local_ts) override;
    virtual void onTimeExpired() override;

  private:
    int seconds_of_day_ = 0;
};

}
}

#endif //TBOX_TIMER_ONESHOT_TIMER_H_20221023
