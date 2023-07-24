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
#include "args.h"

#include <iostream>
#include <fstream>

#include <tbox/base/json.hpp>
#include <tbox/base/version.h>

#include <tbox/util/string.h>
#include <tbox/util/argument_parser.h>
#include <tbox/util/json_deep_loader.h>

namespace tbox {
namespace main {

using namespace std;

string GetAppDescribe();
string GetAppBuildTime();
void GetAppVersion(int &major, int &minor, int &rev, int &build);

Args::Args(Json &conf) :
    conf_(conf)
{ }

bool Args::parse(int argc, const char * const * const argv)
{
    bool run = true;    //!< 是否需要正常运行
    bool print_help = false;    //!< 是否需要打印帮助
    bool print_tips = false;    //!< 是否需要打印Tips
    bool print_cfg  = false;    //!< 是否需要打印配置数据
    bool print_ver  = false;    //!< 是否需要打印配置版本信息
    const string proc_name = argv[0];

    using namespace tbox::util;

    ArgumentParser parser(
        [&] (char short_option, const std::string &long_option, ArgumentParser::OptionValue &option_value) {
            if (short_option == 'h' || long_option == "help") {
                print_help = true;
                run = false;
            } else if (short_option == 'v' || long_option == "version") {
                print_ver = true;
                run = false;
            } else if (short_option == 'n') {
                run = false;
            } else if (short_option == 'p') {
                print_cfg = true;
            } else if (short_option == 'c') {
                if (!option_value.valid()) {
                    cerr << "Error: missing argument to `"<< short_option << "'" << endl;
                    print_tips = true;
                    run = false;
                    return false;
                }
                if (!load(option_value.get())) {
                    print_tips = true;
                    run = false;
                    return false;
                }
            } else if (short_option == 's') {
                if (!option_value.valid()) {
                    cerr << "Error: missing argument to `"<< short_option << "'" << endl;
                    print_tips = true;
                    run = false;
                    return false;
                }
                if (!set(option_value.get())) {
                    print_tips = true;
                    run = false;
                    return false;
                }
            } else {
                //! DO NOTHING
            }

            return true;
        }
    );

    parser.parse(argc, argv);

    if (print_tips)
        printTips(proc_name);

    if (print_help)
        printHelp(proc_name);

    if (print_ver)
        printVersion();

    if (print_cfg)
        cout << conf_.dump(2) << endl;

    return run;
}

void Args::printTips(const std::string &proc_name)
{
    cout << "Try '" << proc_name << " --help' for more information." << endl;
}

void Args::printHelp(const std::string &proc_name)
{
    cout << "Usage: " << proc_name << " [OPTION]" << endl
        << GetAppDescribe() << endl << endl
        << "OPTION" << endl
        << "  -h, --help        display this help text and exit" << endl
        << "  -v, --version     display version and exit" << endl
        << "  -c FILE           specify config file, which content format: JSON" << endl
        << "  -s KEY=VALUE      set config field" << endl
        << "  -p                display config" << endl
        << "  -n                don't run" << endl
        << endl
        << "EXAMPLE" << endl
        << "  " << proc_name << endl
        << "  " << proc_name << R"( -c somewhere/conf.json)" << endl
        << "  " << proc_name << R"( -c somewhere/conf.json -p)" << endl
        << "  " << proc_name << R"( -c somewhere/conf.json -pn)" << endl
        << "  " << proc_name << R"( -s 'log.filelog.enable=true' -s 'log.filelog.path="/tmp/"')" << endl
        << "  " << proc_name << R"( -s 'log.filelog={"enable":true,"path":"/tmp/"}')" << endl
        << "  " << proc_name << R"( -c somewhere/conf.json -s 'thread_pool.min=2')" << endl
        << endl;
}

void Args::printVersion()
{
    int major, minor, rev, build;
    GetTboxVersion(major, minor, rev);
    cout << "tbox version : " << major << '.' << minor << '.' << rev << endl;

    GetAppVersion(major, minor, rev, build);
    cout << " app version : " << major << '.' << minor << '.' << rev << '_' << build << endl
         << "  build time : " << GetAppBuildTime() << endl;

}

bool Args::load(const std::string &config_filename)
{
    try {
        auto js_patch = util::json::LoadDeeply(config_filename);
        conf_.merge_patch(js_patch);
    } catch (const exception &e) {
        cerr << "Error: parse config fail, " << e.what() << endl;
        return false;
    }

    return true;
}

/**
 * 根据 key 生成对应的 Json 补丁对象，并返回后终的 Json 对象引用
 *
 * 如:
 *  key 为 "x.y.z" 时，则生成 Json 对象：
 *  {
 *    "x":{
 *      "y":{
 *        "z":{}
 *      }
 *    }
 *  }
 *  并返回 z 的 Json 对象引用
 */
Json& BuildJsonByKey(const std::string &key, Json &js_root)
{
    vector<string> str_vec;
    util::string::Split(key, ".", str_vec);

    Json *p_node = &js_root;
    //! 为什么取地址，而不是引用？
    //! 因为 Json::operator= 被重写了的，js_node = js_node[str] 会覆盖原内容
    for (auto str : str_vec)
        p_node = &((*p_node)[str]);

    return *p_node;
}

bool Args::set(const std::string &set_string)
{
    vector<string> str_vec;
    if (util::string::Split(set_string, "=", str_vec) != 2) {
        cerr << "Error: invalid argument to `-s', argument format: '<KEY>=<VALUE>'" << endl;
        return false;
    }
    std::string key   = util::string::Strip(str_vec[0]);
    std::string value = util::string::Strip(str_vec[1]);

    if (key.empty() || value.empty()) {
        cerr << "Error: KEY or VALUE is empty";
        return false;
    }

    Json js_patch = Json::object();
    try {
        Json &js_node = BuildJsonByKey(key, js_patch);
        js_node = Json::parse(value);
    } catch (const exception &e) {
        cerr << "Error: parse KEY=VALUE fail, " << e.what() << endl;
        return false;
    }

    conf_.merge_patch(js_patch);
    return true;
}

}
}
