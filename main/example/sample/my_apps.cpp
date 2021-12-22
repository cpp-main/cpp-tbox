#include <vector>
#include <tbox/base/log.h>
#include <tbox/main/app.h>
#include <tbox/main/context.h>

class MyApp1 : public tbox::main::App
{
  public:
    MyApp1()
    {
        LogTag();
    }

    ~MyApp1()
    {
        LogTag();
    }

    bool initialize() override
    {
        LogTag();
        return true;
    }

    bool start() override
    {
        LogTag();
        return true;
    }

    void stop() override
    {
        LogTag();
    }

    void cleanup() override
    {
        LogTag();
    }
};

class MyApp2 : public tbox::main::App
{
  public:
    MyApp2()
    {
        LogTag();
    }
    ~MyApp2()
    {
        LogTag();
    }

    bool initialize() override
    {
        LogTag();
        return true;
    }

    bool start() override
    {
        LogTag();
        return true;
    }

    void stop() override
    {
        LogTag();
    }

    void cleanup() override
    {
        LogTag();
    }
};

namespace tbox {
namespace main {
void RegisterApps(Context &context, std::vector<App*> &apps)
{
    apps.push_back(new MyApp1);
    apps.push_back(new MyApp2);
}
}
}
