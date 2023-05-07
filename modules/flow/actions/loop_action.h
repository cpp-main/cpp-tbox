#ifndef TBOX_FLOW_LOOP_ACTION_H_20221017
#define TBOX_FLOW_LOOP_ACTION_H_20221017

#include "../action.h"

namespace tbox {
namespace flow {

class LoopAction : public Action {
  public:
    enum class Mode {
        kNone,
        kForever,     //! while(true) { action() };
        kUntilFail,   //! while(action());
        kUntilSucc,   //! while(!action());
    };

    explicit LoopAction(event::Loop &loop);

    void setMode(Mode mode);
    void setChild(Action::SharedPtr child);

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
    Mode mode_ = Mode::kNone;
};

}
}

#endif //TBOX_FLOW_LOOP_ACTION_H_20221017
