#ifndef TBOX_LOG_ASYNC_SYSLOG_SINK_H_20220408
#define TBOX_LOG_ASYNC_SYSLOG_SINK_H_20220408

#include "async_sink.h"

namespace tbox {
namespace log {

class AsyncSyslogSink : public AsyncSink {
  public:
    AsyncSyslogSink();

  protected:
    virtual void appendLog(const char *str, size_t len) override;
};

}
}

#endif //TBOX_LOG_ASYNC_SYSLOG_SINK_H_20220408
