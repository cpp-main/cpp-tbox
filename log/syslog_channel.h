#ifndef TBOX_LOG_OUTPUT_SYSLOG_H_20220408
#define TBOX_LOG_OUTPUT_SYSLOG_H_20220408

#include "channel.h"

namespace tbox {
namespace log {

class SyslogChannel : public Channel {
  protected:
    virtual void onLogFrontEnd(const void *data_ptr, size_t data_size);
};

}
}

#endif //TBOX_LOG_OUTPUT_SYSLOG_H_20220408
