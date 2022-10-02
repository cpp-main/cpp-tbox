#include "executor.h"
#include "event_publisher_impl.h"

namespace tbox {
namespace action {

class ExecutorImpl : public Context {
  public:
    explicit ExecutorImpl(event::Loop &loop);
    virtual ~ExecutorImpl();

  public:
    virtual event::Loop& loop() override { return loop_; }
    virtual EventPublisher& event_publisher() override { return *event_publisher_; }

    void onEvent(int event_id, void *event_data) {
      event_publisher_->onEvent(event_id, event_data);
    }

  private:
    event::Loop &loop_;
    EventPublisherImpl *event_publisher_ = nullptr;
};

Executor::Executor(event::Loop &loop) : impl_(new ExecutorImpl(loop)) { }

Executor::~Executor() { delete impl_; }

Context& Executor::context() { return *impl_; }

void Executor::onEvent(int event_id, void *event_data) {
  impl_->onEvent(event_id, event_data);
}

}
}
