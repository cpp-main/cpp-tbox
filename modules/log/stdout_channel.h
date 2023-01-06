#ifndef TBOX_LOG_OUTPUT_STDOUT_H_20220408
#define TBOX_LOG_OUTPUT_STDOUT_H_20220408

#include "channel.h"

namespace tbox {
namespace log {

class StdoutChannel : public Channel {
  protected:
    virtual void onLogFrontEnd(const void *data_ptr, size_t data_size);
};

}
}

#endif //TBOX_LOG_OUTPUT_STDOUT_H_20220408
