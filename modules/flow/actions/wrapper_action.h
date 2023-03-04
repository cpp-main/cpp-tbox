#ifndef TBOX_FLOW_WRAPPER_ACTION_H_20221105
#define TBOX_FLOW_WRAPPER_ACTION_H_20221105

#include "../action.h"

namespace tbox {
namespace flow {

/// 包装动作类，对子Action的结果进行处理
class WrapperAction : public Action {
  public:
    enum class Mode {
        kNormal,    //!< 正常，child
        kInvert,    //!< 取反，!child
        kAlwaySucc, //!< 总是成功，true
        kAlwayFail, //!< 总是失败，false
    };

  public:
    explicit WrapperAction(event::Loop &loop, Action *child, Mode mode = Mode::kNormal);
    virtual ~WrapperAction();

    Mode mode() const { return mode_; }

  protected:
    virtual void toJson(Json &js) const override;

    virtual bool onStart() override;
    virtual bool onStop() override;
    virtual bool onPause() override;
    virtual bool onResume() override;
    virtual void onReset() override;

  private:
    Action *child_;
    Mode mode_;
};

std::string ToString(WrapperAction::Mode mode);

}
}

#endif //TBOX_FLOW_WRAPPER_ACTION_H_20221105
