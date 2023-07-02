#include <tbox/main/main.h>
#include <tbox/base/log.h>
#include <tbox/base/catch_throw.h>
#include <tbox/util/argument_parser.h>

#include <dlfcn.h>

namespace tbox {
namespace {

using PluginRegisterAppsFunc = void(*)(main::Module &, main::Context &);

std::vector<PluginRegisterAppsFunc> _register_func_vec;

std::vector<std::string> GetPluginFileList(int argc, char **argv)
{
    std::vector<std::string> plugin_file_vec;
    util::ArgumentParser parser(
        [&] (char short_option, const std::string &long_option,
             util::ArgumentParser::OptionValue &option_value) {
            if (long_option == "plugin")
                plugin_file_vec.push_back(option_value.get());
            return true;
        }
    );

    parser.parse(argc, argv);
    return plugin_file_vec;
}

}

void LoadPlugins(int argc, char **argv)
{
    auto plugin_file_vec = GetPluginFileList(argc, argv);

    for (auto &plugin_file : plugin_file_vec) {
        void *dl_handle = dlopen(plugin_file.c_str(), RTLD_NOW);
        if (dl_handle == nullptr)
            continue;

        auto register_func = (PluginRegisterAppsFunc)dlsym(dl_handle, "PluginRegisterAppsFunc");
        if (register_func == nullptr) {
            dlclose(dl_handle);
            continue;
        }

        _register_func_vec.push_back(register_func);
    }
}

namespace main {

void RegisterApps(Module &apps, Context &ctx)
{
    for (auto func : _register_func_vec)
        func(apps, ctx);
}

std::string GetAppDescribe() { return "plugin runner"; }

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
