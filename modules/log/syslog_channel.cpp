#include "syslog_channel.h"
#include <syslog.h>

namespace tbox {
namespace log {

void SyslogChannel::writeLog(const char *str, size_t len)
{
    syslog(LOG_INFO, "%s", str);
}

}
}
