#ifndef TBOX_FLOW_REPEAT_ACTION_H_20221017
#define TBOX_FLOW_REPEAT_ACTION_H_20221017

#include "../action.h"

namespace tbox {
namespace flow {

class RepeatAction : public Action {
  public:
    enum class Mode {
      kNoBreak,     //! for (int i = 0; i < times; ++i) { action() };
      kBreakFail,   //! for (int i = 0; i < times && action(); ++i);
      kBreakSucc,   //! for (int i = 0; i < times && !action(); ++i);
    };

    explicit RepeatAction(event::Loop &loop);

    void setChild(Action::SharedPtr child);
    void setMode(Mode mode);
    void setRepeatTimes(size_t repeat_times);

    virtual void toJson(Json &js) const override;

  protected:
    virtual bool onInit() override;
    virtual bool onStart() override;
    virtual bool onStop() override;
    virtual bool onPause() override;
    virtual bool onResume() override;
    virtual void onReset() override;

  private:
    Action::SharedPtr child_;
    size_t repeat_times_ = 0;
    size_t remain_times_ = 0;
    Mode mode_ = Mode::kNoBreak;
};

}
}

#endif //TBOX_FLOW_REPEAT_ACTION_H_20221017
