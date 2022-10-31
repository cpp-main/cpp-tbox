#ifndef TBOX_ACTION_ACTION_H_20221001
#define TBOX_ACTION_ACTION_H_20221001

#include <string>
#include <functional>

#include <tbox/base/json_fwd.h>
#include <tbox/event/forward.h>

namespace tbox {
namespace action {

class Action {
  public:
    explicit Action(event::Loop &loop, const std::string &id);
    virtual ~Action();

  public:
    enum class Status {
      kIdle,
      kRunning,
      kPause,
      kFinished
    };

    enum class Result {
      kUnsure,
      kSuccess,
      kFail,
    };

    virtual std::string type() const = 0;

    std::string id() const { return id_; }
    Status status() const { return status_; }
    Result result() const { return result_; }

    using FinishCallback = std::function<void(bool is_succ)>;
    void setFinishCallback(const FinishCallback &cb) { finish_cb_ = cb; }

    virtual void toJson(Json &js) const;

    bool start();
    bool pause();
    bool resume();
    bool stop();

    static std::string ToString(Status status);

  protected:
    bool finish(bool is_succ);

    virtual bool onStart() { return true; }
    virtual bool onPause() { return true; }
    virtual bool onResume() { return true; }
    virtual bool onStop() { return true; }
    virtual void onFinished(bool is_succ) { }

  protected:
    event::Loop &loop_;

  private:
    std::string id_;
    FinishCallback finish_cb_;

    Status status_ = Status::kIdle;
    Result result_ = Result::kUnsure;
};

}
}

#endif //TBOX_ACTION_ACTION_H_20221001
