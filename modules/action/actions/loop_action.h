#ifndef TBOX_ACTION_LOOP_ACTION_H_20221017
#define TBOX_ACTION_LOOP_ACTION_H_20221017

#include "../action.h"

namespace tbox {
namespace action {

class LoopAction : public Action {
  public:
    enum class Mode {
      kForever,     //! while(true) { action() };
      kUntilFail,   //! while(action());
      kUntilSucc,   //! while(!action());
    };
    explicit LoopAction(Context &ctx, const std::string &name, Action *child,
        Mode mode = Mode::kForever);
    virtual ~LoopAction();

    virtual std::string type() const override { return "Loop"; }
    virtual void toJson(Json &js) const;

    virtual bool start() override;
    virtual bool pause() override;
    virtual bool resume() override;
    virtual bool stop() override;

  private:
    Action *child_;
    Mode mode_;
};

}
}

#endif //TBOX_ACTION_LOOP_ACTION_H_20221017
