#include "file_async_channel.h"

#include <unistd.h>
#include <iostream>
#include <sstream>
#include <tbox/util/fs.h>

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
    proc_name_ = proc_name;
    log_path_ = log_path;
    pid_ = ::getpid();

    //!FIXME:要对log_path进行规范化

    AsyncChannel::Config cfg;
    cfg.buff_size = 1024;
    cfg.buff_num  = 3;
    cfg.interval  = 100;

    return AsyncChannel::initialize(cfg);
}

void FileAsyncChannel::cleanup()
{
    AsyncChannel::cleanup();
    pid_ = 0;
}

void FileAsyncChannel::onLogBackEnd(const std::string &log_text)
{
    if (!checkAndCreateLogFile())
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
    if (!util::fs::MakeDirectory(log_path_)) {
        cerr << "Err: create director " << log_path_ << " fail." << endl;
        return false;
    }

    char timestamp[16];
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
