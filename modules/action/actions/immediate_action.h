#ifndef TBOX_ACTION_IMMEDIATE_ACTION_H_20221003
#define TBOX_ACTION_IMMEDIATE_ACTION_H_20221003

#include "../action.h"

namespace tbox {
namespace action {

class ImmediateAction : public Action {
  public:
    using Func = std::function<bool()>;
    explicit ImmediateAction(Context &ctx, const Func &func) :
      Action(ctx), func_(func) { }

    virtual std::string type() const override { return "Immediate"; }

    virtual bool start() override {
      Action::start();
      finish(func_());
      return true;
    }

  private:
    Func func_;
};

}
}

#endif //TBOX_ACTION_IMMEDIATE_ACTION_H_20221003
