#include <tbox/main/main.h>

#include "app1/app.h"
#include "app2/app.h"
#include "app3/app.h"

namespace tbox {
namespace main {

void RegisterApps(Apps &apps)
{
    apps.add(new app1::App);
    apps.add(new app2::App);
    apps.add(new app3::App);
}

std::string GetAppDescribe()
{
    return "There apps sample";
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
