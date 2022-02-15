#include <tbox/main/main.h>

namespace tbox::main {
std::string GetAppBuildTime()
{
    return __DATE__ " " __TIME__;
}
}
