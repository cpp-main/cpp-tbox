#include "log_imp.h"
#include "log.h"

#include <sys/time.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <vector>
#include <algorithm>
#include <mutex>

#ifdef __cplusplus
extern "C" {
#endif

static std::mutex _lock;
static uint32_t _id_alloc = 0;
static std::vector<std::pair<uint32_t, LogPrintfFuncType>> _output_channels;

/**
 * \brief   日志格式化打印接口的实现
 *
 * 1.对数据合法性进行校验;
 * 2.将日志数据打包成 LogContent，然后调用 _log_printf_func 指向的函数进行输出
 */
void LogPrintfFunc(const char *module_id, const char *func_name, const char *file_name,
                   int line, int level, bool with_args, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    if (level < 0) level = 0;
    if (level > LOG_LEVEL_TRACE) level = LOG_LEVEL_TRACE;

    const char *module_id_be_print = (module_id != nullptr) ? module_id : "???";

    struct timeval tv;
    struct timezone tz;
    gettimeofday(&tv, &tz);

    LogContent content = {
        .thread_id = syscall(SYS_gettid),
        .timestamp = {
            .sec  = static_cast<uint32_t>(tv.tv_sec),
            .usec = static_cast<uint32_t>(tv.tv_usec),
        },
        .module_id = module_id_be_print,
        .func_name = func_name,
        .file_name = file_name,
        .line = line,
        .level = level,
        .with_args = with_args,
        .fmt = fmt
    };

    va_copy(content.args, args);    //! va_list 不能直接赋值，需要使用 va_copy()
    {
        std::lock_guard<std::mutex> lg(_lock);
        for (const auto &item : _output_channels) {
            if (item.second)
                item.second(&content);
        }
    }
    va_end(args);
}

uint32_t LogAddPrintfFunc(const LogPrintfFuncType &func)
{
    std::lock_guard<std::mutex> lg(_lock);
    uint32_t new_id = ++_id_alloc;
    _output_channels.push_back(std::make_pair(new_id, func));
    return new_id;
}

bool LogRemovePrintfFunc(uint32_t id)
{
    std::lock_guard<std::mutex> lg(_lock);
    auto iter = std::remove_if(_output_channels.begin(), _output_channels.end(),
        [id](const std::pair<uint32_t, LogPrintfFuncType> item) {
            return (item.first == id);
        }
    );

    if (iter != _output_channels.end()) {
        _output_channels.erase(iter, _output_channels.end());
        return true;
    }
    return false;
}

#ifdef __cplusplus
}
#endif
