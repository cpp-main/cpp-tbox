#ifndef TBOX_ASYNC_H_20230507
#define TBOX_ASYNC_H_20230507

#include <string>
#include <vector>

#include <tbox/eventx/thread_pool.h>

namespace tbox {
namespace async {

class Async {
  public:
    explicit Async(eventx::ThreadPool &thread_pool);

  public:
    using ErrorCodeCb = std::function<void(int)>;
    using ErrorCodeStringCb = std::function<void(int, std::string &)>;
    using ErrorCodeStringVecCb = std::function<void(int, std::vector<std::string> &)>;

    void readFile(const std::string &filename, ErrorCodeStringCb &&cb);
    void readFileLines(const std::string &filename, ErrorCodeStringVecCb &&cb);

    void writeFile(const std::string &filename, const std::string &context, bool sync_now, ErrorCodeCb &&cb);
    void appendFile(const std::string &filename, const std::string &context, bool sync_now, ErrorCodeCb &&cb);

    void executeCmd(const std::string &filename, const std::string &context, ErrorCodeCb &&cb);
    void executeCmd(const std::string &filename, const std::string &context, ErrorCodeStringCb &&cb);

  private:
    eventx::ThreadPool &thread_pool_;
};

}
}

#endif //TBOX_ASYNC_H_20230507
