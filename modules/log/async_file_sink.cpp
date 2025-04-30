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
    while (file_path_.length() >= 2 && file_path_.back() == '/')
        file_path_.pop_back();
    //! Q: 为什么这里是 file_path_.length() >= 2 而不是 !file_path_.empty()
    //! A: 为了防止 "/" 这样的情况。如果这种情况也去除了，那就变成空字串了

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
    sym_filepath_ = file_path_ + '/' + file_prefix_ + '.' + "latest.log";
    CHECK_CLOSE_RESET_FD(fd_);
}

void AsyncFileSink::endline()
{
    cache_.push_back('\n');
}

void AsyncFileSink::flush()
{
    if (pid_ == 0 || !checkAndCreateLogFile())
        return;

    auto wsize = ::write(fd_, cache_.data(), cache_.size());
    if (wsize != static_cast<ssize_t>(cache_.size())) {
        cerr << "Err: write file error." << endl;
        return;
    }

    total_write_size_ += cache_.size();

    if (total_write_size_ >= file_max_size_)
        CHECK_CLOSE_RESET_FD(fd_);

    cache_.clear();
}

bool AsyncFileSink::checkAndCreateLogFile()
{
    if (fd_ >= 0) {
        if (util::fs::IsFileExist(log_filepath_))
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

    //! 生成新的日志文件名与路径
    std::string log_filename; //! 日志文件名称，如：demo.20250402.123.log
    std::string log_filepath; //! 日志文件路径，如：/var/log/demo.20250402.123.log
    int postfix = 0;

    do {
        log_filename = file_prefix_ + '.' + timestamp + '.' + std::to_string(pid_) + ".log";
        if (postfix != 0) {
            log_filename += '.';
            log_filename += std::to_string(postfix);
        }
        log_filepath = file_path_ + '/' + log_filename;
        ++postfix;
    } while (util::fs::IsFileExist(log_filepath));    //! 避免在同一秒多次创建日志文件，都指向同一日志名

    log_filepath_ = std::move(log_filepath);

    int flags = O_CREAT | O_WRONLY | O_APPEND;
    if (file_sync_enable_)
        flags |= O_DSYNC;

    fd_ = ::open(log_filepath_.c_str(), flags, S_IRUSR | S_IWUSR);
    if (fd_ < 0) {
        cerr << "Err: open file " << log_filepath_ << " fail. error:" << errno << ',' << strerror(errno) << endl;
        return false;
    }

    total_write_size_ = 0;
    util::fs::RemoveFile(sym_filepath_, false);
    util::fs::MakeSymbolLink(log_filename, sym_filepath_, false);

    return true;
}

}
}
