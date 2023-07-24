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
#include "async_file_sink.h"

#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <tbox/base/defines.h>
#include <tbox/util/fs.h>
#include <tbox/util/string.h>

namespace tbox {
namespace log {

using namespace std;

AsyncFileSink::AsyncFileSink()
{
    AsyncSink::Config cfg;
    cfg.buff_size = 10240;
    cfg.buff_min_num = 2;
    cfg.buff_max_num = 20;
    cfg.interval  = 100;

    setConfig(cfg);
    pid_ = ::getpid();
}

AsyncFileSink::~AsyncFileSink()
{
    if (pid_ != 0)
        cleanup();
}

void AsyncFileSink::setFilePath(const std::string &file_path)
{
    file_path_ = util::string::Strip(file_path);

    //! 确保最后没有'/'字符。如果最后一个字符是/，就弹出
    if (file_path_.length() > 1 &&
        file_path_.at(file_path_.length() - 1) == '/')
        file_path_.pop_back();

    updateInnerValues();
}

void AsyncFileSink::setFilePrefix(const std::string &file_prefix)
{
    file_prefix_ = util::string::Strip(file_prefix);
    updateInnerValues();
}

void AsyncFileSink::setFileSyncEnable(bool enable)
{
    file_sync_enable_ = enable;
    CHECK_CLOSE_RESET_FD(fd_);
}

void AsyncFileSink::cleanup()
{
    AsyncSink::cleanup();
    pid_ = 0;
}

void AsyncFileSink::updateInnerValues()
{
    filename_prefix_ = file_path_ + '/' + file_prefix_ + '.';
    sym_filename_ = filename_prefix_ + "latest.log";

    CHECK_CLOSE_RESET_FD(fd_);
}

void AsyncFileSink::appendLog(const char *str, size_t len)
{
    buffer_.reserve(buffer_.size() + len - 1);
    std::back_insert_iterator<std::vector<char>>  back_insert_iter(buffer_);
    std::copy(str, str + len - 1, back_insert_iter);
}

void AsyncFileSink::flushLog()
{
    if (pid_ == 0 || !checkAndCreateLogFile())
        return;

    auto wsize = ::write(fd_, buffer_.data(), buffer_.size());
    if (wsize != static_cast<ssize_t>(buffer_.size())) {
        cerr << "Err: write file error." << endl;
        return;
    }

    total_write_size_ += buffer_.size();

    buffer_.clear();

    if (total_write_size_ >= file_max_size_)
        CHECK_CLOSE_RESET_FD(fd_);
}

bool AsyncFileSink::checkAndCreateLogFile()
{
    if (fd_ >= 0) {
        if (util::fs::IsFileExist(log_filename_))
            return true;
        CHECK_CLOSE_RESET_FD(fd_);
    }

    //!检查并创建路径
    if (!util::fs::MakeDirectory(file_path_, false)) {
        cerr << "Err: create director " << file_path_ << " fail. error:" << errno << ',' << strerror(errno) << endl;
        return false;
    }

    char timestamp[16]; //! 固定长度16B，"20220414_071023"
    {
        time_t ts_sec = time(nullptr);
        struct tm tm;
        localtime_r(&ts_sec, &tm);
        strftime(timestamp, sizeof(timestamp), "%Y%m%d_%H%M%S", &tm);
    }

    std::string log_filename;
    int postfix = 0;
    do {
        log_filename = filename_prefix_ + timestamp + '.' + std::to_string(pid_) + ".log";
        if (postfix != 0) {
            log_filename += '.';
            log_filename += std::to_string(postfix);
        }
        ++postfix;
    } while (util::fs::IsFileExist(log_filename));    //! 避免在同一秒多次创建日志文件，都指向同一日志名
    log_filename_ = std::move(log_filename);

    int flags = O_CREAT | O_WRONLY | O_APPEND;
    if (file_sync_enable_)
        flags |= O_DSYNC;

    fd_ = ::open(log_filename_.c_str(), flags, S_IRUSR | S_IWUSR);
    if (fd_ < 0) {
        cerr << "Err: open file " << log_filename_ << " fail. error:" << errno << ',' << strerror(errno) << endl;
        return false;
    }

    total_write_size_ = 0;
    util::fs::RemoveFile(sym_filename_, false);
    util::fs::MakeSymbolLink(log_filename_, sym_filename_, false);

    return true;
}

}
}
