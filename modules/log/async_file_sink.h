/*
 *     .============.
 *    //  M A K E  / \
 *   //  C++ DEV  /   \
 *  //  E A S Y  /  \/ \
 * ++ ----------.  \/\  .
 *  \\     \     \ /\  /
 *   \\     \     \   /
 *    \\     \     \ /
 *     -============'
 *
 * Copyright (c) 2018 Hevake and contributors, all rights reserved.
 *
 * This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
 * Use of this source code is governed by MIT license that can be found
 * in the LICENSE file in the root of the source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */
#ifndef TBOX_LOG_ASYNC_FILE_SINK_H_20220412
#define TBOX_LOG_ASYNC_FILE_SINK_H_20220412

#include "async_sink.h"

#include <vector>

namespace tbox {
namespace log {

class AsyncFileSink : public AsyncSink {
  public:
    AsyncFileSink();
    virtual ~AsyncFileSink() override;

  public:
    void cleanup();

    void setFilePath(const std::string &file_path);
    void setFilePrefix(const std::string &file_path);
    void setFileMaxSize(size_t max_size) { file_max_size_ = max_size; }
    void setFileSyncEnable(bool enable);
    std::string currentFilePath() const { return log_filepath_; }

  protected:
    void updateInnerValues();

    virtual void endline() override;
    virtual void flush() override;

    bool checkAndCreateLogFile();

  private:
    std::string file_prefix_ = "none";
    std::string file_path_ = "/var/log/";
    size_t file_max_size_ = (1 << 20);  //!< 默认文件大小为1MB
    bool file_sync_enable_ = false;
    pid_t pid_ = 0;

    std::string log_filepath_;
    std::string sym_filepath_;

    std::vector<char> buffer_;

    int fd_ = -1;
    size_t total_write_size_ = 0;
};

}
}

#endif //TBOX_LOG_ASYNC_FILE_SINK_H_20220412
