#ifndef TBOX_ACTION_IMMEDIATE_ACTION_H_20221002
#define TBOX_ACTION_IMMEDIATE_ACTION_H_20221002

#include "../action.h"
#include <tbox/base/json.hpp>

namespace tbox {
namespace action {

class ImmediateAction : Action {
  public:
    ImmediateAction(Context &ctx, bool is_done) :
      Action(ctx), is_done_(is_done) { }

    virtual std::string type() const override { return "Immediate"; }

    virtual void toJson(Json &js) const override {
      Action::toJson(js);
      js["is_done"] = is_done_;
    }

    virtual bool start() override { return finish(is_done_); }

  private:
    bool is_done_;
};

}
}

#endif //TBOX_ACTION_IMMEDIATE_ACTION_H_20221002
