#include "channel.h"
#include <cstring>
#include <functional>
#include <iostream>

#define LOG_MAX_LEN (100 << 10)     //! 限定单条日志最大长度

namespace tbox {
namespace log {

Channel::~Channel()
{
    disable();
}

void Channel::setLevel(const std::string &module, int level)
{
    std::lock_guard<std::mutex> _lk(lock_);
    if (module.empty())
        default_level_ = level;
    else
        modules_level_[module] = level;
}

void Channel::unsetLevel(const std::string &module)
{
    modules_level_.erase(module);
}

void Channel::enableColor(bool enable)
{
    enable_color_ = enable;
}

bool Channel::enable()
{
    using namespace std::placeholders;
    if (output_id_ == 0) {
        onEnable();
        output_id_ = LogAddPrintfFunc(HandleLog, this);
        return true;
    }
    return false;
}

void Channel::disable()
{
    if (output_id_ != 0) {
        LogRemovePrintfFunc(output_id_);
        onDisable();
        output_id_ = 0;
    }
}

bool Channel::filter(int level, const std::string &module)
{
    std::lock_guard<std::mutex> _lk(lock_);
    auto iter = modules_level_.find(module);
    if (iter != modules_level_.end())
        return level <= iter->second;
    else
        return level <= default_level_;
}

void Channel::HandleLog(const LogContent *content, void *ptr)
{
    Channel *pthis = static_cast<Channel*>(ptr);
    pthis->handleLog(content);
}

void Channel::handleLog(const LogContent *content)
{
    if (!filter(content->level, content->module_id))
        return;

    onLogFrontEnd(content);
}

}
}
