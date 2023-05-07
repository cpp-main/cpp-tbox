#ifndef TBOX_FLOW_SUCC_FAIL_ACTION_H_20230304
#define TBOX_FLOW_SUCC_FAIL_ACTION_H_20230304

#include "../action.h"

namespace tbox {
namespace flow {

/// 成功动作
class SuccAction : public Action {
  public:
    explicit SuccAction(event::Loop &loop) : Action(loop, "Succ") { }

  protected:
    virtual bool onInit() override { return true; }
    virtual bool onStart() {
        loop_.runNext([this] { finish(true); });
        return true;
    }
};

/// 失败动作
class FailAction : public Action {
  public:
    explicit FailAction(event::Loop &loop) : Action(loop, "Fail") { }

  protected:
    virtual bool onInit() override { return true; }
    virtual bool onStart() {
        loop_.runNext([this] { finish(false); });
        return true;
    }
};

}
}

#endif //TBOX_FLOW_SUCC_FAIL_ACTION_H_20230304
