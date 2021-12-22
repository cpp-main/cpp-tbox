/**
 * pid 文件类
 *
 * 通常一个服务程序在运行时会创建一个pid文件。
 * pid 文件有两个使用场景：
 * 1. 记录当前服务程序的进程号，以便其它程序能给该进程发送信号
 * 2. 防止多个服务进程同时运行，导致资源抢占异常
 *
 * 该 PidFile 类使用比较简单
 *
 * int main() {
 *   PidFile pid("/var/my_server.pid");
 *   if (!pid.lock()) {
 *     //! 获取不到 pid 文件的写权限，应当主动结束进程
 *     return 0;
 *   }
 *
 *   //! 正常的服务业务逻辑
 * }
 */
#ifndef TBOX_PID_FILE_H_20211221
#define TBOX_PID_FILE_H_20211221

#include <string>
#include <tbox/base/defines.h>

namespace tbox {
namespace util {

class PidFile {
  public:
    explicit PidFile(const std::string &pid_filename);
    virtual ~PidFile();

    NONCOPYABLE(PidFile);

    bool lock();
    bool unlock();

  private:
    std::string pid_filename_;
    int fd_ = -1;
};

}
}

#endif //TBOX_PID_FILE_H_20211221
