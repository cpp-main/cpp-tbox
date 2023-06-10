#ifndef TBOX_SYSTEM_H_20230507
#define TBOX_SYSTEM_H_20230507

#include <string>
#include <vector>

#include <tbox/eventx/thread_pool.h>

namespace tbox {
namespace system {

/// 将与系统相关的阻塞性调用转换成异步回调形式
class System {
  public:
    explicit System(eventx::ThreadPool *thread_pool);

  public:
    using Callback = std::function<void(int)>;
    using StringCallback = std::function<void(int, std::string &)>;
    using StringVecCallback = std::function<void(int, std::vector<std::string> &)>;

    void readFile(const std::string &filename, StringCallback &&cb);
    void readFileLines(const std::string &filename, StringVecCallback &&cb);

    void writeFile (const std::string &filename, const std::string &context, bool sync_now = false, Callback &&cb = nullptr);
    void appendFile(const std::string &filename, const std::string &context, bool sync_now = false, Callback &&cb = nullptr);

    void removeFile(const std::string &filename, Callback &&cb = nullptr);

    void executeCmd(const std::string &cmd, Callback &&cb = nullptr);
    void executeCmd(const std::string &cmd, StringCallback &&cb);

  private:
    eventx::ThreadPool *thread_pool_;
};

}
}

#endif //TBOX_SYSTEM_H_20230507
