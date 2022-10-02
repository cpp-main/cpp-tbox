#ifndef TBOX_ACTION_ACTION_H_20221001
#define TBOX_ACTION_ACTION_H_20221001

#include <string>
#include <functional>

#include "event_subscriber.h"

namespace tbox {
namespace action {

class Context;

class Action : public EventSubscriber {
  public:
    explicit Action(Context &ctx);
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
    Status status() const { return status_; }

    using FinishCallback = std::function<void(bool is_done)>;
    void setFinishCallback(const FinishCallback &cb) { finish_cb_ = cb; }

    virtual bool start();
    virtual bool pause();
    virtual bool resume();
    virtual bool stop();

  protected:
    bool finish(bool is_succ);

  private:
    Context &ctx_;
    Status status_ = Status::kIdle;
    FinishCallback finish_cb_;
};

}
}

#endif //TBOX_ACTION_ACTION_H_20221001
