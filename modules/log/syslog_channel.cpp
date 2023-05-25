#include "syslog_channel.h"
#include <syslog.h>

namespace tbox {
namespace log {

SyslogChannel::SyslogChannel()
{
    AsyncChannel::Config cfg;
    cfg.buff_size = 10240;
    cfg.buff_min_num = 2;
    cfg.buff_max_num = 20;
    cfg.interval  = 100;

    setConfig(cfg);
}

void SyslogChannel::appendLog(const char *str, size_t len)
{
    syslog(LOG_INFO, "%s", str);
}

}
}
