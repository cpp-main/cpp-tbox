#ifndef TBOX_LOG_FILE_CHANNEL_H_20220412
#define TBOX_LOG_FILE_CHANNEL_H_20220412

#include "async_channel.h"

#include <vector>

namespace tbox {
namespace log {

class FilelogChannel : public AsyncChannel {
  public:
    virtual ~FilelogChannel() override;

  public:
    bool initialize();
    void cleanup();

    void setFilePath(const std::string &file_path);
    void setFilePrefix(const std::string &file_path);
    void setFileMaxSize(size_t max_size) { file_max_size_ = max_size; }
    std::string currentFilename() const { return log_filename_; }

  protected:
    void updateInnerValues();

    virtual void appendLog(const char *str, size_t len) override;
    virtual void flushLog() override;

    bool checkAndCreateLogFile();

  private:
    std::string file_prefix_ = "none";
    std::string file_path_ = "/var/log/";
    size_t file_max_size_ = (1 << 20);  //!< 默认文件大小为1MB
    pid_t pid_ = 0;

    std::string filename_prefix_;
    std::string sym_filename_;
    std::string log_filename_;

    std::vector<char> buffer_;

    int fd_ = -1;
    size_t total_write_size_ = 0;
};

}
}

#endif //TBOX_LOG_FILE_CHANNEL_H_20220412
