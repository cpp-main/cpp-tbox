#include <tbox/main/main.h>

#include "app1/app.h"
#include "app2/app.h"

namespace tbox::main {
void RegisterApps(Apps &apps)
{
    apps.add(new app1::App);
    apps.add(new app2::App);
}

std::string GetAppDescribe()
{
    return "This is a sample within two apps.";
}

void GetAppVersion(int &major, int &minor, int &rev, int &build)
{
    major = 1;
    minor = 0;
    rev = 0;
    build = 0;
}

}
