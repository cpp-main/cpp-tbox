#include <tbox/main/main.h>
#include <tbox/base/log.h>
#include <tbox/base/log_output.h>
#include <tbox/base/catch_throw.h>
#include <tbox/util/argument_parser.h>
#include <tbox/util/fs.h>

#include <iostream>
#include <dlfcn.h>

namespace {

typedef void(*PluginRegisterAppsFunc) (tbox::main::Module &, tbox::main::Context &);

std::vector<void*> _dl_handle_vec;
std::vector<PluginRegisterAppsFunc> _register_func_vec;

void ParseArgs(int argc, char **argv,
               std::vector<std::string> &plugin_file_vec)
{
    tbox::util::ArgumentParser parser(
        [&] (char short_option, const std::string &long_option,
             tbox::util::ArgumentParser::OptionValue &option_value) {
            if (long_option == "plugin")
                plugin_file_vec.push_back(option_value.get());
            return true;
        }
    );

    parser.parse(argc, argv);
}

/// 导入插件动态库
void Load(int argc, char **argv)
{
    std::vector<std::string> plugin_file_vec;
    ParseArgs(argc, argv, plugin_file_vec);

    for (auto &plugin_file : plugin_file_vec) {
        if (!tbox::util::fs::IsFileExist(plugin_file)) {
            LogWarn("file %s not exist.", plugin_file.c_str());
            continue;
        }

        void *dl_handle = dlopen(plugin_file.c_str(), RTLD_NOW);
        if (dl_handle == nullptr) {
            LogWarn("load %s faild.", plugin_file.c_str());
            const char *err_str = dlerror();
            if (err_str != nullptr)
                LogNotice("reason: %s", err_str);
            continue;
        }

        auto register_func = (PluginRegisterAppsFunc)dlsym(dl_handle, "PluginRegisterApps");
        if (register_func == nullptr) {
            dlclose(dl_handle);
            LogWarn("can't find 'PluginRegisterApps' symbol is %s.", plugin_file.c_str());
            continue;
        }

        _dl_handle_vec.push_back(dl_handle);
        _register_func_vec.push_back(register_func);

        LogInfo("load %s success.", plugin_file.c_str());
    }
}

/// 关闭插件动态库
void Release()
{
    for (void *dl_handle : _dl_handle_vec)
        ::dlclose(dl_handle);
    _dl_handle_vec.clear();
}

}

namespace tbox {
namespace main {

void RegisterApps(Module &apps, Context &ctx)
{
    for (auto func : _register_func_vec)
        func(apps, ctx);
    _register_func_vec.clear();
}

std::string GetAppDescribe() { return "plugin runner"; }

std::string GetAppBuildTime() { return __DATE__ " " __TIME__; }

void GetAppVersion(int &major, int &minor, int &rev, int &build)
{
    major = 0;
    minor = 0;
    rev = 1;
    build = 0;
}

}
}

int main(int argc, char **argv)
{
    LogOutput_Enable();
    Load(argc, argv);
    LogOutput_Disable();

    auto ret = tbox::main::Main(argc, argv);

    Release();
    return ret;
}
