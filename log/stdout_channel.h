#ifndef TBOX_LOG_OUTPUT_STDOUT_H_20220408
#define TBOX_LOG_OUTPUT_STDOUT_H_20220408

#include "channel.h"

namespace tbox {
namespace log {

class StdoutChannel : public Channel {
  public:
    void enableColor(bool enable) { enable_color_ = enable; }

  protected:
    virtual void onLogFrontEnd(LogContent *content) override;

  private:
    bool enable_color_ = true;
};

}
}

#endif //TBOX_LOG_OUTPUT_STDOUT_H_20220408
