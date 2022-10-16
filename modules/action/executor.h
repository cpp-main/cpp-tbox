#ifndef TBOX_ACTION_EXECUTOR_H_20221002
#define TBOX_ACTION_EXECUTOR_H_20221002

#include "context.h"

namespace tbox {
namespace action {

class Context;
class ExecutorImpl;

class Executor {
  public:
    explicit Executor(event::Loop &loop);
    virtual ~Executor();

  public:
    Context& context();
    void onEvent(Event event);

  private:
    ExecutorImpl *impl_;
};

}
}

#endif //TBOX_ACTION_EXECUTOR_H_20221002
