/*
 *     .============.
 *    //  M A K E  / \
 *   //  C++ DEV  /   \
 *  //  E A S Y  /  \/ \
 * ++ ----------.  \/\  .
 *  \\     \     \ /\  /
 *   \\     \     \   /
 *    \\     \     \ /
 *     -============'
 *
 * Copyright (c) 2018 Hevake and contributors, all rights reserved.
 *
 * This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
 * Use of this source code is governed by MIT license that can be found
 * in the LICENSE file in the root of the source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */
#include <tbox/main/main.h>
#include <tbox/base/log.h>
#include <tbox/base/log_output.h>
#include <tbox/util/argument_parser.h>
#include <tbox/util/fs.h>

#include <iostream>
#include <dlfcn.h>

namespace {

typedef void(*RegisterAppsFunc) (tbox::main::Module &, tbox::main::Context &);

std::vector<void*> _dl_handle_vec;
std::vector<RegisterAppsFunc> _register_func_vec;

/// 解决参数中的'-l <module>' 与 '--load <module>' 参数作为模块动态库列表
void ParseArgs(int argc, char **argv,
               std::vector<std::string> &module_file_vec)
{
    tbox::util::ArgumentParser parser(
        [&] (char short_option, const std::string &long_option,
             tbox::util::ArgumentParser::OptionValue &option_value) {
            if (short_option == 'l' || long_option == "load")
                module_file_vec.push_back(option_value.get());
            return true;
        }
    );

    parser.parse(argc, argv);
}

/// 导入模块动态库
void Load(int argc, char **argv)
{
    std::vector<std::string> module_file_vec;
    ParseArgs(argc, argv, module_file_vec);

    for (auto &module_file : module_file_vec) {
        if (!tbox::util::fs::IsFileExist(module_file)) {
            LogWarn("file %s not exist.", module_file.c_str());
            continue;
        }

        void *dl_handle = dlopen(module_file.c_str(), RTLD_NOW);
        if (dl_handle == nullptr) {
            LogWarn("load %s faild.", module_file.c_str());
            const char *err_str = dlerror();
            if (err_str != nullptr)
                LogNotice("reason: %s", err_str);
            continue;
        }

        auto register_func = (RegisterAppsFunc)dlsym(dl_handle, "RegisterApps");
        if (register_func == nullptr) {
            dlclose(dl_handle);
            LogWarn("can't find 'RegisterApps' symbol in %s.", module_file.c_str());
            continue;
        }

        _dl_handle_vec.push_back(dl_handle);
        _register_func_vec.push_back(register_func);

        LogInfo("load %s success.", module_file.c_str());
    }
}

/// 关闭模块动态库
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
    if (_register_func_vec.empty()) {
        LogWarn("no module");
        return;
    }

    for (auto register_func : _register_func_vec)
        register_func(apps, ctx);
    _register_func_vec.clear();
}

std::string GetAppDescribe() { return "modules runner"; }

std::string GetAppBuildTime() { return __DATE__ " " __TIME__; }

void GetAppVersion(int &major, int &minor, int &rev, int &build)
{
    major = 0;
    minor = 0;
    rev = 2;
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
