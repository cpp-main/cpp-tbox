#include "syslog_channel.h"
#include <syslog.h>

namespace tbox {
namespace log {

bool SyslogChannel::initialize()
{
    AsyncChannel::Config cfg;
    cfg.buff_size = 10240;
    cfg.buff_min_num = 2;
    cfg.buff_max_num = 20;
    cfg.interval  = 100;

    return AsyncChannel::initialize(cfg);
}

void SyslogChannel::writeLog(const char *str, size_t len)
{
    syslog(LOG_INFO, "%s", str);
}

}
}
