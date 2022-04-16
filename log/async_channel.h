#ifndef TBOX_LOG_ASYNC_CHANNEL_H_20220408
#define TBOX_LOG_ASYNC_CHANNEL_H_20220408

#include "channel.h"
#include <tbox/util/async_pipe.h>

namespace tbox {
namespace log {

#define TIMESTAMP_STRING_SIZE   20

class AsyncChannel : public Channel {
  public:
    AsyncChannel();
    virtual ~AsyncChannel() override;

    using Config = util::AsyncPipe::Config;
    bool initialize(const Config &cfg);
    void cleanup();

    void enableColor(bool enable) { enable_color_ = enable; }

  protected:
    virtual void onLogFrontEnd(LogContent *content) override;
    virtual void onLogBackEnd(const std::string &log_text) = 0;

  private:
    void onPipeAppend(const void *data_ptr, size_t data_size);
    void udpateTimestampStr(uint32_t sec);

  private:
    util::AsyncPipe async_pipe_;
    std::string string_buff_;

    bool enable_color_ = false;
    uint32_t timestamp_sec_ = 0;
    char timestamp_str_[TIMESTAMP_STRING_SIZE]; //!2022-04-12 14:33:30
};

}
}

#endif //TBOX_LOG_ASYNC_CHANNEL_H_20220408
