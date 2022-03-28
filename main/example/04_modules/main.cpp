#include <tbox/main/main.h>
#include "app.h"

namespace tbox {
namespace main {

void RegisterApps(Apps &apps)
{
    apps.add(new ::App);
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
