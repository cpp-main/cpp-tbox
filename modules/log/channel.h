#ifndef TBOX_LOGOUTPUT_CHANNEL_H_20220406
#define TBOX_LOGOUTPUT_CHANNEL_H_20220406

#include <map>
#include <string>
#include <mutex>
#include <tbox/base/log.h>
#include <tbox/base/log_imp.h>

#define TIMESTAMP_STRING_SIZE   15

namespace tbox {
namespace log {

//! 日志打印通道类
class Channel {
  public:
    virtual ~Channel();

    void setLevel(int level, const std::string &module = "");
    void enableColor(bool enable);

    bool enable();
    void disable();

  protected:
    virtual void onEnable() { }
    virtual void onDisable() { }

    virtual void onLogFrontEnd(const void *data_ptr, size_t data_size) = 0;

    void handleLog(const LogContent *content);
    void udpateTimestampStr(uint32_t sec);

  private:
    static void HandleLog(const LogContent *content, void *ptr);
    bool filter(int level, const std::string &module);

  private:
    std::mutex lock_;

    uint32_t output_id_ = 0;
    std::map<std::string, int> modules_level_;
    int default_level_ = LOG_LEVEL_INFO;

    bool enable_color_ = false;
    uint32_t timestamp_sec_ = 0;
    char timestamp_str_[TIMESTAMP_STRING_SIZE]; //!2022-04-12 14:33:30
};

}
}

#endif //TBOX_LOGOUTPUT_CHANNEL_H_20220406
