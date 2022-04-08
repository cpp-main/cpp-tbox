#ifndef TBOX_LOGOUTPUT_CHANNEL_H_20220406
#define TBOX_LOGOUTPUT_CHANNEL_H_20220406

#include <map>
#include <string>
#include <mutex>
#include <tbox/base/log.h>
#include <tbox/base/log_imp.h>

namespace tbox {
namespace log_output {

//! 日志打印通道类
class Channel {
  public:
    virtual ~Channel();

    void setLevel(int level, const std::string &module = "");

    bool enable();
    void disable();

  protected:
    virtual void onEnable() { }
    virtual void onDisable() { }
    virtual void onLog(LogContent *content) = 0;    //!< 需要自已去实现

    std::mutex lock_;

  private:
    static void HandleLog(LogContent *content, void *ptr);
    bool filter(int level, const std::string &module);

    uint32_t output_id_ = 0;
    std::map<std::string, int> modules_level_;
    int default_level_ = LOG_LEVEL_INFO;
};

}
}

#endif //TBOX_LOGOUTPUT_CHANNEL_H_20220406
