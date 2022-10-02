#ifndef TBOX_ACTION_SLEEP_FOR_ACTION_H_20221002
#define TBOX_ACTION_SLEEP_FOR_ACTION_H_20221002

#include "../action.h"
#include <chrono>
#include <tbox/event/forward.h>

namespace tbox {
namespace action {

class SleepForAction : public Action {
  public:
    SleepForAction(Context &ctx, const std::chrono::milliseconds &time_span, bool is_done = true);
    ~SleepForAction();

    virtual std::string type() const override { return "SleepFor"; }
    virtual void toJson(Json &js) const override;

    virtual bool start() override;
    virtual bool stop() override;

  private:
    event::TimerEvent *timer_;
    bool is_done_;
};

}
}

#endif //TBOX_ACTION_SLEEP_FOR_ACTION_H_20221002
