#include "channel.h"
#include <functional>

namespace tbox {
namespace log_output {

Channel::~Channel()
{
    disable();
}

void Channel::setLevel(int level, const std::string &module)
{
    std::lock_guard<std::mutex> _lk(lock_);
    if (module.empty())
        default_level_ = level;
    else
        modules_level_[module] = level;
}

bool Channel::enable()
{
    using namespace std::placeholders;
    if (output_id_ == 0) {
        output_id_ = LogAddPrintfFunc(HandleLog, this);
        onEnable();
        return true;
    }
    return false;
}

void Channel::disable()
{
    if (output_id_ != 0) {
        onDisable();
        LogRemovePrintfFunc(output_id_);
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

void Channel::HandleLog(LogContent *content, void *ptr)
{
    Channel *pthis = static_cast<Channel*>(ptr);
    if (pthis->filter(content->level, content->module_id))
        pthis->onLog(content);
}

}
}
