#include "syslog_channel.h"
#include <syslog.h>

namespace tbox {
namespace log {

void SyslogChannel::onLogFrontEnd(const void *data_ptr, size_t data_size)
{
    const char *str_ptr = static_cast<const char *>(data_ptr);
    syslog(LOG_INFO, "%s", str_ptr);
}

}
}
