#include "file_async_channel.h"

#include <unistd.h>
#include <iostream>
#include <sstream>
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
    log_prefix_ = util::string::Strip(log_prefix);
    log_path_ = util::string::Strip(log_path);

    //! 如果最后一个字符是/，就弹出
    if (log_path_.at(log_path_.length() - 1) == '/')
        log_path_.pop_back();

    if (log_prefix_.empty() || log_path_.empty())
        return false;

    AsyncChannel::Config cfg;
    cfg.buff_size = 1024;
    cfg.buff_min_num = 2;
    cfg.buff_max_num = 20;
    cfg.interval  = 100;

    bool ok = AsyncChannel::initialize(cfg);
    if (ok) {
        pid_ = ::getpid();
        return true;
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

    ofs_ << log_text << endl << flush;

    if (ofs_.tellp() >= file_max_size_)
        ofs_.close();
}

bool FileAsyncChannel::checkAndCreateLogFile()
{
    if (ofs_.is_open())
        return true;

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

    std::string filename;
    int postfix = 0;
    do {
        ostringstream filename_oss;
        filename_oss << log_path_ << '/' << log_prefix_ << '_' << pid_ << '.' << timestamp << ".log";
        if (postfix != 0)
            filename_oss << "." << postfix;
        filename = filename_oss.str();
        ++postfix;
    } while (util::fs::IsFileExist(filename));    //! 避免在同一秒多次创建日志文件，都指向同一日志名

    ofs_.open(filename, ofstream::out | ofstream::app);
    return ofs_.is_open();
}

}
}
