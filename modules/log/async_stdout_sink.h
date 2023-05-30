#ifndef TBOX_LOG_ASYNC_STDOUT_SINK_H_20220408
#define TBOX_LOG_ASYNC_STDOUT_SINK_H_20220408

#include "async_sink.h"

#include <vector>

namespace tbox {
namespace log {

class AsyncStdoutSink : public AsyncSink {
  public:
    AsyncStdoutSink();

  protected:
    virtual void appendLog(const char *str, size_t len) override;
    virtual void flushLog() override;

  private:
    std::vector<char> buffer_;
};

}
}

#endif //TBOX_LOG_ASYNC_STDOUT_SINK_H_20220408
