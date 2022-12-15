#ifndef TBOX_FLOW_SEQUENCE_FLOW_H_20221002
#define TBOX_FLOW_SEQUENCE_FLOW_H_20221002

#include "../action.h"

namespace tbox {
namespace flow {

/**
 * bool SequenceAction(acton_vec, finish_condition) {
 *   for (item : action_vec) {
 *     auto is_succ = item();
 *     if ((finish_condition == AnySucc && is_succ) ||
 *         (finish_condition == AnyFail && !is_succ))
 *       return is_succ;
 *   }
 *   return true;
 * }
 */
class SequenceAction : public Action {
  public:
    //! 结束条件
    enum class FinishCondition {
      kAllFinish, //!< 全部结束
      kAnyFail,   //!< 任一失败
      kAnySucc,   //!< 任一成功
    };

  public:
    using Action::Action;
    virtual ~SequenceAction();

    virtual std::string type() const override { return "Sequence"; }

    virtual void toJson(Json &js) const;

    int append(Action *action);

    inline void setFinishCondition(FinishCondition finish_condition) {
      finish_condition_ = finish_condition;
    }

    inline int index() const { return index_; }

  protected:
    virtual bool onStart() override;
    virtual bool onStop() override;
    virtual bool onPause() override;
    virtual bool onResume() override;
    virtual void onReset() override;

  private:
    void startOtheriseFinish();
    void onChildFinished(bool is_succ);

  private:
    FinishCondition finish_condition_ = FinishCondition::kAllFinish;
    size_t index_ = 0;
    std::vector<Action*> children_;
};

}
}

#endif //TBOX_FLOW_SEQUENCE_FLOW_H_20221002
