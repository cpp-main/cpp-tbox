#include "file_async_channel.h"

#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <iostream>
#include <sstream>
#include <tbox/base/defines.h>
#include <tbox/util/fs.h>
#include <tbox/util/string.h>

namespace tbox {
namespace log {

using namespace std;

FileAsyncChannel::~FileAsyncChannel()
{
    if (pid_ != 0)
        cleanup();
}

bool FileAsyncChannel::initialize(const std::string &log_path, const std::string &log_prefix)
{
    log_path_ = util::string::Strip(log_path);

    //! 确保最后没有'/'字符。如果最后一个字符是/，就弹出
    if (log_path_.length() > 1 &&
        log_path_.at(log_path_.length() - 1) == '/')
        log_path_.pop_back();

    auto log_prefix_strip = util::string::Strip(log_prefix);
    if (log_prefix_strip.empty() || log_path_.empty()) {
        cerr << "Err: prefix or log_path is empty" << endl;
        return false;
    }

    filename_prefix_ = log_path_ + '/' + log_prefix_strip + '.';
    sym_filename_ = filename_prefix_ + "latest.log";

    AsyncChannel::Config cfg;
    cfg.buff_size = 1024;
    cfg.buff_min_num = 2;
    cfg.buff_max_num = 20;
    cfg.interval  = 100;

    bool ok = AsyncChannel::initialize(cfg);
    if (ok) {
        pid_ = ::getpid();
        return checkAndCreateLogFile();
    }
    return false;
}

void FileAsyncChannel::cleanup()
{
    AsyncChannel::cleanup();
    pid_ = 0;
}

void FileAsyncChannel::onLogBackEnd(const std::string &log_text)
{
    if (pid_ == 0 || !checkAndCreateLogFile())
        return;

    const char endline = '\n';
    ::write(fd_, log_text.c_str(), log_text.size());
    ::write(fd_, &endline, 1);
    ::fsync(fd_);

    total_write_size_ += log_text.size() + 1;

    if (total_write_size_ >= file_max_size_)
        CHECK_CLOSE_RESET_FD(fd_);
}

bool FileAsyncChannel::checkAndCreateLogFile()
{
    if (fd_ >= 0) {
        if (util::fs::IsFileExist(log_filename_))
            return true;
        CHECK_CLOSE_RESET_FD(fd_);
    }

    //!检查并创建路径
    if (!util::fs::MakeDirectory(log_path_, false)) {
        cerr << "Err: create director " << log_path_ << " fail." << endl;
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

    fd_ = ::open(log_filename_.c_str(), O_CREAT | O_WRONLY | O_APPEND, S_IRUSR | S_IWUSR);
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
