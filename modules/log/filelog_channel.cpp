#include "filelog_channel.h"

#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <base/defines.h>
#include <util/fs.h>
#include <util/string.h>

namespace tbox {
namespace log {

using namespace std;

FilelogChannel::FilelogChannel()
{
    AsyncChannel::Config cfg;
    cfg.buff_size = 10240;
    cfg.buff_min_num = 2;
    cfg.buff_max_num = 20;
    cfg.interval  = 100;

    setConfig(cfg);
    pid_ = ::getpid();
}

FilelogChannel::~FilelogChannel()
{
    if (pid_ != 0)
        cleanup();
}

void FilelogChannel::setFilePath(const std::string &file_path)
{
    file_path_ = util::string::Strip(file_path);

    //! 确保最后没有'/'字符。如果最后一个字符是/，就弹出
    if (file_path_.length() > 1 &&
        file_path_.at(file_path_.length() - 1) == '/')
        file_path_.pop_back();

    updateInnerValues();
}

void FilelogChannel::setFilePrefix(const std::string &file_prefix)
{
    file_prefix_ = util::string::Strip(file_prefix);
    updateInnerValues();
}

void FilelogChannel::cleanup()
{
    AsyncChannel::cleanup();
    pid_ = 0;
}

void FilelogChannel::updateInnerValues()
{
    filename_prefix_ = file_path_ + '/' + file_prefix_ + '.';
    sym_filename_ = filename_prefix_ + "latest.log";

    CHECK_CLOSE_RESET_FD(fd_);
}

void FilelogChannel::appendLog(const char *str, size_t len)
{
    buffer_.reserve(buffer_.size() + len - 1);
    std::back_insert_iterator<std::vector<char>>  back_insert_iter(buffer_);
    std::copy(str, str + len - 1, back_insert_iter);
}

void FilelogChannel::flushLog()
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
    if (buffer_.capacity() > 1024)
        buffer_.shrink_to_fit();

    if (total_write_size_ >= file_max_size_)
        CHECK_CLOSE_RESET_FD(fd_);
}

bool FilelogChannel::checkAndCreateLogFile()
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
        ostringstream filename_oss;
        filename_oss << filename_prefix_ << timestamp << '.' << pid_ << ".log";
        if (postfix != 0)
            filename_oss << "." << postfix;
        log_filename = filename_oss.str();
        ++postfix;
    } while (util::fs::IsFileExist(log_filename));    //! 避免在同一秒多次创建日志文件，都指向同一日志名
    log_filename_ = log_filename;

    fd_ = ::open(log_filename_.c_str(), O_CREAT | O_WRONLY | O_APPEND | O_DSYNC, S_IRUSR | S_IWUSR);
    if (fd_ < 0) {
        cerr << "Err: open file " << log_filename_ << " fail. error:" << errno << ',' << strerror(errno) << endl;
        return false;
    }

    total_write_size_ = 0;
    util::fs::RemoveFile(sym_filename_, false);
    util::fs::MakeSymbolLink(log_filename, sym_filename_, false);

    return true;
}

}
}
