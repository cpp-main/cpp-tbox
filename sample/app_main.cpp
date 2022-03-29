#include <tbox/main/main.h>

#include "app1/app.h"
#include "app2/app.h"

namespace tbox {
namespace main {

void RegisterApps(Module &apps, Context &ctx)
{
    apps.add(new app1::App(ctx));
    apps.add(new app2::App(ctx));
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
}
