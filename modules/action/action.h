#ifndef TBOX_ACTION_ACTION_H_20221001
#define TBOX_ACTION_ACTION_H_20221001

#include <string>
#include <functional>

#include <tbox/base/json_fwd.h>

#include "context.h"
#include "event_subscriber.h"

namespace tbox {
namespace action {

class Action : public EventSubscriber {
  public:
    explicit Action(Context &ctx, const std::string &name);
    virtual ~Action();

  public:
    enum class Status {
      kIdle,
      kRunning,
      kPause,
      kDone,
      kFail
    };

    virtual std::string type() const = 0;
    std::string name() const { return name_; }
    Status status() const { return status_; }

    using FinishCallback = std::function<void(bool is_done)>;
    void setFinishCallback(const FinishCallback &cb) { finish_cb_ = cb; }

    virtual void toJson(Json &js) const;

    virtual bool start();
    virtual bool pause();
    virtual bool resume();
    virtual bool stop();

    virtual bool onEvent(int event_id, void *event_data) override;

    static std::string ToString(Status status);

  protected:
    bool finish(bool is_succ);

  protected:
    Context &ctx_;

  private:
    std::string name_;
    Status status_ = Status::kIdle;
    FinishCallback finish_cb_;
};

}
}

#endif //TBOX_ACTION_ACTION_H_20221001
