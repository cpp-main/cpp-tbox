#include <main/main.h>

#include "echo_server/app.h"
#include "nc_client/app.h"

namespace tbox {
namespace main {

void RegisterApps(Module &apps, Context &ctx)
{
    apps.add(new echo_server::App(ctx));
    apps.add(new nc_client::App(ctx));
}

std::string GetAppDescribe()
{
    return "There are two app. One is nc_client, the other is echo_server";
}

std::string GetAppBuildTime()
{
    return __DATE__ " " __TIME__;
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
