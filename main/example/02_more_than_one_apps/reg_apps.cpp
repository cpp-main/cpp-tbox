#include <tbox/main/main.h>

#include "app1/app.h"
#include "app2/app.h"
#include "app3/app.h"

namespace tbox::main {
void RegisterApps(Context &context, Apps &apps)
{
    apps.add(new app1::App(context));
    apps.add(new app2::App(context));
    apps.add(new app3::App(context));
}
}
