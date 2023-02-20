#ifndef TBOX_FLOW_LOOP_ACTION_H_20221017
#define TBOX_FLOW_LOOP_ACTION_H_20221017

#include "../action.h"

namespace tbox {
namespace flow {

class LoopAction : public Action {
  public:
    enum class Mode {
      kForever,     //! while(true) { action() };
      kUntilFail,   //! while(action());
      kUntilSucc,   //! while(!action());
    };
    explicit LoopAction(event::Loop &loop, Action *child, Mode mode = Mode::kForever);
    virtual ~LoopAction();

    virtual void toJson(Json &js) const;

  protected:
    virtual bool onStart() override;
    virtual bool onStop() override;
    virtual bool onPause() override;
    virtual bool onResume() override;
    virtual void onReset() override;

  private:
    Action *child_;
    Mode mode_;
};

}
}

#endif //TBOX_FLOW_LOOP_ACTION_H_20221017
