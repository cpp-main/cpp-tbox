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

bool FileAsyncChannel::initialize(const std::string &proc_name, const std::string &log_path)
{
    proc_name_ = util::string::Strip(proc_name);
    log_path_ = util::string::Strip(log_path);

    //! 如果最后一个字符是/，就弹出
    if (log_path_.at(log_path_.length() - 1) == '/')
        log_path_.pop_back();

    if (proc_name_.empty() || log_path_.empty())
        return false;

    AsyncChannel::Config cfg;
    cfg.buff_size = 1024;
    cfg.buff_num  = 3;
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

    ostringstream filename_oss;
    filename_oss << log_path_ << '/' << proc_name_ << '_' << pid_ << '.' << timestamp << ".log";
    ofs_.open(filename_oss.str(), ofstream::out | ofstream::app);

    return ofs_.is_open();
}

}
}
