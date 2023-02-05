#ifndef TBOX_LOG_ASYNC_CHANNEL_H_20220408
#define TBOX_LOG_ASYNC_CHANNEL_H_20220408

#include "channel.h"
#include <tbox/util/async_pipe.h>

namespace tbox {
namespace log {

class AsyncChannel : public Channel {
  public:
    AsyncChannel();
    virtual ~AsyncChannel() override;

    using Config = util::AsyncPipe::Config;
    bool initialize(const Config &cfg);
    void cleanup();

  protected:
    virtual void onLogFrontEnd(const void *data_ptr, size_t data_size) override;
    virtual void onLogBackEnd(const void *data_ptr, size_t data_size) = 0;

  private:
    util::AsyncPipe async_pipe_;
};

}
}

#endif //TBOX_LOG_ASYNC_CHANNEL_H_20220408
