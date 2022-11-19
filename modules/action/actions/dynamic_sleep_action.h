#ifndef TBOX_ACTION_DYNAMIC_SLEEP_ACTION_H_20221119
#define TBOX_ACTION_DYNAMIC_SLEEP_ACTION_H_20221119

#include "../action.h"
#include <chrono>

namespace tbox {
namespace action {

/**
 * \brief 动态延时动作
 *
 * 与SleepAction对比，DynamicSleepAction的延时的时长不是在构造时就指定的。
 * DynamicSleepAction在构建的时候没有提供固定的延时长度，则是提供了一个可以返回一个
 * 时间长度的函数对象Generator。
 * 在开始延时的时候，通过调用构造时传入的gen函数获取要延时的时间长度。
 * 达到同一个DynamicSleepAction对象，在不同的时间点启动时，延迟的长度可不同的效果。
 */
class DynamicSleepAction : public Action {
  public:
    using Generator = std::function<std::chrono::milliseconds ()>;

    DynamicSleepAction(event::Loop &loop, const Generator &gen);
    ~DynamicSleepAction();

    virtual std::string type() const override { return "DynamicSleep"; }

  protected:
    virtual bool onStart() override;
    virtual bool onStop() override;
    virtual bool onPause() override;
    virtual bool onResume() override;
    virtual void onReset() override;

  private:
    event::TimerEvent *timer_;
    Generator gen_;
};

}
}

#endif //TBOX_ACTION_DYNAMIC_SLEEP_ACTION_H_20221119
