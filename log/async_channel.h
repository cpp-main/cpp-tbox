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

  protected:
    virtual void onLogFrontEnd(LogContent *content) override;
    virtual void onLogBackEnd(const std::string &log_text) = 0;

  private:
    void onPipeAppend(const void *data_ptr, size_t data_size);

  private:
    util::AsyncPipe async_pipe_;
    std::string string_buff_;
};

}
}

#endif //TBOX_LOG_ASYNC_CHANNEL_H_20220408
