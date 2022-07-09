#include <tbox/main/main.h>

namespace tbox {
namespace main {

void RegisterApps(Module &apps, Context &ctx)
{
    static_cast<uint8_t*>(nullptr)[0] = 0;
}

std::string GetAppDescribe()
{
    return "One app sample";
}

std::string GetAppBuildTime()
{
    return __DATE__ " " __TIME__;
}

void GetAppVersion(int &major, int &minor, int &rev, int &build)
{
    major = 0;
    minor = 0;
    rev = 1;
    build = 0;
}

}
}
