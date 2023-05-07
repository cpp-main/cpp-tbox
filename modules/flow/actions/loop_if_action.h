#ifndef TBOX_FLOW_LOOP_IF_ACTION_H_20221108
#define TBOX_FLOW_LOOP_IF_ACTION_H_20221108

#include "../action.h"

namespace tbox {
namespace flow {

class LoopIfAction : public Action {
  public:
    explicit LoopIfAction(event::Loop &loop);

    void setIfAction(Action::SharedPtr action);
    void setExecAction(Action::SharedPtr action);

    virtual void toJson(Json &js) const override;

  protected:
    virtual bool onInit() override;
    virtual bool onStart() override;
    virtual bool onStop() override;
    virtual bool onPause() override;
    virtual bool onResume() override;
    virtual void onReset() override;

  private:
    Action::SharedPtr if_action_;
    Action::SharedPtr exec_action_;
};

}
}

#endif //TBOX_FLOW_LOOP_IF_ACTION_H_20221108
