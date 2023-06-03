#include <syslog.h>
#include "async_syslog_sink.h"

namespace tbox {
namespace log {

AsyncSyslogSink::AsyncSyslogSink()
{
    AsyncSink::Config cfg;
    cfg.buff_size = 10240;
    cfg.buff_min_num = 2;
    cfg.buff_max_num = 20;
    cfg.interval  = 100;

    setConfig(cfg);
}

void AsyncSyslogSink::appendLog(const char *str, size_t len)
{
    syslog(LOG_INFO, "%s", str);
    (void)len;
}

}
}
