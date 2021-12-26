#include <tbox/main/main.h>

#include "echo_server/app.h"
#include "nc_client/app.h"

namespace tbox::main {
void RegisterApps(Context &context, Apps &apps)
{
    apps.add(new echo_server::App(context));
    apps.add(new nc_client::App(context));
}
}
