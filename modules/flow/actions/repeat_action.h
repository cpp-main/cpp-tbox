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

    explicit RepeatAction(event::Loop &loop, Action *child, size_t times,
                          Mode mode = Mode::kNoBreak);
    virtual ~RepeatAction();

    virtual void toJson(Json &js) const override;

  protected:
    virtual bool onStart() override;
    virtual bool onStop() override;
    virtual bool onPause() override;
    virtual bool onResume() override;
    virtual void onReset() override;

  private:
    Action *child_;
    size_t repeat_times_;
    size_t remain_times_ = 0;
    Mode mode_;
};

}
}

#endif //TBOX_FLOW_REPEAT_ACTION_H_20221017
