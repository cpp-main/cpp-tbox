#ifndef TBOX_LOG_ASYNC_CHANNEL_H_20220408
#define TBOX_LOG_ASYNC_CHANNEL_H_20220408

#include "channel.h"

#include <vector>
#include <tbox/util/async_pipe.h>

namespace tbox {
namespace log {

class AsyncChannel : public Channel {
  public:
    using Config = util::AsyncPipe::Config;
    bool initialize(const Config &cfg);
    void cleanup();

  protected:
    virtual void onLogFrontEnd(const LogContent *content) override;
    void onLogBackEndReadPipe(const void *data_ptr, size_t data_size);
    void onLogBackEnd(const LogContent *content);
    void udpateTimestampStr(uint32_t sec);
    virtual void writeLog(const char *str, size_t len) = 0;

  private:
    util::AsyncPipe async_pipe_;

    uint32_t timestamp_sec_ = 0;
    char timestamp_str_[TIMESTAMP_STRING_SIZE]; //!2022-04-12 14:33:30

    std::vector<uint8_t> buffer_;
};

}
}

#endif //TBOX_LOG_ASYNC_CHANNEL_H_20220408
