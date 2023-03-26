#ifndef TBOX_FLOW_LOOP_IF_ACTION_H_20221108
#define TBOX_FLOW_LOOP_IF_ACTION_H_20221108

#include "../action.h"

namespace tbox {
namespace flow {

class LoopIfAction : public Action {
  public:
    explicit LoopIfAction(event::Loop &loop,
                          Action *if_action,
                          Action *exec_action);
    virtual ~LoopIfAction();

    virtual void toJson(Json &js) const;

    //! 默认结束结果是true，如果需要可以设定
    void setFinishResult(bool succ) { finish_result_ = succ; }

  protected:
    virtual bool onStart() override;
    virtual bool onStop() override;
    virtual bool onPause() override;
    virtual bool onResume() override;
    virtual void onReset() override;

  private:
    Action *if_action_;
    Action *exec_action_;
    bool finish_result_ = true;
};

}
}

#endif //TBOX_FLOW_LOOP_IF_ACTION_H_20221108
