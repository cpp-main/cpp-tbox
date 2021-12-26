#include <tbox/main/main.h>
#include "app.h"

namespace tbox::main {
void RegisterApps(Context &context, Apps &apps)
{
    apps.add(new ::App(context));
}
}
