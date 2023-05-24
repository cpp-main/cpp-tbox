#ifndef TBOX_LOG_OUTPUT_STDOUT_H_20220408
#define TBOX_LOG_OUTPUT_STDOUT_H_20220408

#include "async_channel.h"

#include <vector>

namespace tbox {
namespace log {

class StdoutChannel : public AsyncChannel {
  public:
    bool initialize();

  protected:
    virtual void appendLog(const char *str, size_t len) override;
    virtual void flushLog() override;

  private:
    std::vector<char> buffer_;
};

}
}

#endif //TBOX_LOG_OUTPUT_STDOUT_H_20220408
