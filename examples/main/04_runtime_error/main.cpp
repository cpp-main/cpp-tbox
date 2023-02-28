#include <tbox/main/main.h>

namespace tbox {
namespace main {

class TestModule : public Module {
  public:
    TestModule(Context &ctx) :
        Module("test", ctx)
    { }

    virtual bool onStart() {
        ctx().loop()->run(
            [this] {
                static_cast<uint8_t*>(nullptr)[0] = 0;
            }
        );
        return true;
    }
};

void RegisterApps(Module &apps, Context &ctx)
{
    apps.add(new TestModule(ctx));
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
    rev = 2;
    build = 0;
}

}
}
