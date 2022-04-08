#ifndef TBOX_LOG_ASYNC_CHANNEL_H_20220408
#define TBOX_LOG_ASYNC_CHANNEL_H_20220408

#include "channel.h"

namespace tbox {
namespace log {

class AsyncChannel : public Channel {
  public:
    AsyncChannel();
    virtual ~AsyncChannel() override;

  protected:
    virtual void onLogFrontEnd(LogContent *content) override;
    virtual void onLogBackEnd(const std::string &log_text) = 0;
};

}
}

#endif //TBOX_LOG_ASYNC_CHANNEL_H_20220408
