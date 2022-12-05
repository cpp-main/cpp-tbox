#ifndef TBOX_ACTION_LOOP_ACTION_H_20221017
#define TBOX_ACTION_LOOP_ACTION_H_20221017

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

    virtual std::string type() const override { return "Loop"; }
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

#endif //TBOX_ACTION_LOOP_ACTION_H_20221017
