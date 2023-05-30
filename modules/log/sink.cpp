#include "sink.h"

#include <cstring>
#include <functional>
#include <iostream>

#define LOG_MAX_LEN (100 << 10)     //! 限定单条日志最大长度

namespace tbox {
namespace log {

Sink::~Sink()
{
    disable();
}

void Sink::setLevel(int level)
{
    std::lock_guard<std::mutex> _lk(lock_);
    default_level_ = level;
}

void Sink::setLevel(const std::string &module, int level)
{
    std::lock_guard<std::mutex> _lk(lock_);
    if (module.empty())
        default_level_ = level;
    else
        modules_level_[module] = level;
}

void Sink::unsetLevel(const std::string &module)
{
    modules_level_.erase(module);
}

void Sink::enableColor(bool enable)
{
    enable_color_ = enable;
}

bool Sink::enable()
{
    using namespace std::placeholders;
    if (output_id_ == 0) {
        onEnable();
        output_id_ = LogAddPrintfFunc(HandleLog, this);
        return true;
    }
    return false;
}

void Sink::disable()
{
    if (output_id_ != 0) {
        LogRemovePrintfFunc(output_id_);
        onDisable();
        output_id_ = 0;
    }
}

bool Sink::filter(int level, const std::string &module)
{
    std::lock_guard<std::mutex> _lk(lock_);
    auto iter = modules_level_.find(module);
    if (iter != modules_level_.end())
        return level <= iter->second;
    else
        return level <= default_level_;
}

void Sink::HandleLog(const LogContent *content, void *ptr)
{
    Sink *pthis = static_cast<Sink*>(ptr);
    pthis->handleLog(content);
}

void Sink::handleLog(const LogContent *content)
{
    if (!filter(content->level, content->module_id))
        return;

    onLogFrontEnd(content);
}

}
}
