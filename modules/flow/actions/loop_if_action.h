#ifndef TBOX_FLOW_LOOP_IF_FLOW_H_20221108
#define TBOX_FLOW_LOOP_IF_FLOW_H_20221108

#include "../action.h"

namespace tbox {
namespace flow {

class LoopIfAction : public Action {
  public:
    explicit LoopIfAction(event::Loop &loop,
                          Action *cond_action,
                          Action *exec_action);
    virtual ~LoopIfAction();

    virtual std::string type() const override { return "LoopIf"; }
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
    Action *cond_action_;
    Action *exec_action_;
    bool is_cond_done_ = false;
    bool finish_result_ = true;
};

}
}

#endif //TBOX_FLOW_LOOP_IF_FLOW_H_20221108
