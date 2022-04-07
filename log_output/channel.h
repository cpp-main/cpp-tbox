#ifndef TBOX_LOGOUTPUT_CHANNEL_H_20220406
#define TBOX_LOGOUTPUT_CHANNEL_H_20220406

namespace tbox {
namespace log_output {

//! 日志打印通道类
class Channel {
  public:
    void setLevel(int level, const std::string &module = "");
    bool enable();
    void disable();

  protected:
    void onLog();
};

}
}

#endif //TBOX_LOGOUTPUT_CHANNEL_H_20220406
