#ifndef TBOX_LOG_ASYNC_CHANNEL_H_20220408
#define TBOX_LOG_ASYNC_CHANNEL_H_20220408

#include "channel.h"

#include <vector>
#include <util/async_pipe.h>

namespace tbox {
namespace log {

class AsyncChannel : public Channel {
  public:
    using Config = util::AsyncPipe::Config;

    void setConfig(const Config &cfg) { cfg_ = cfg; }
    void cleanup();

  protected:
    virtual void onEnable() override;
    virtual void onDisable() override;

    virtual void onLogFrontEnd(const LogContent *content) override;
    void onLogBackEndReadPipe(const void *data_ptr, size_t data_size);
    void onLogBackEnd(const LogContent *content);
    void udpateTimestampStr(uint32_t sec);
    virtual void appendLog(const char *str, size_t len) = 0;
    virtual void flushLog() { }

  private:
    Config cfg_;
    util::AsyncPipe async_pipe_;
    bool is_pipe_inited_ = false;

    uint32_t timestamp_sec_ = 0;
    char timestamp_str_[TIMESTAMP_STRING_SIZE]; //!2022-04-12 14:33:30

    std::vector<char> buffer_;
};

}
}

#endif //TBOX_LOG_ASYNC_CHANNEL_H_20220408
