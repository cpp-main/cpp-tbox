#include "args.h"

#include <iostream>
#include <fstream>

#include <tbox/base/json.hpp>

#include <tbox/util/string.h>

namespace tbox::main {

using namespace std;

std::string GetAppDescribe();
std::string GetAppBuildTime();
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
    std::string proc_name;

    for (int i = 0; i < argc; ++i) {
        const std::string curr = argv[i];
        const std::string next = (argc == (i + 1)) ? "" : argv[i + 1];
        if (i == 0) {
            proc_name = curr;
        } else if (curr == "-h" || curr == "--help") {
            print_help = true;
            run = false;
        } else if (curr == "-v" || curr == "--version") {
            print_ver = true;
            run = false;
        } else if (curr == "-n") {
            run = false;
        } else if (curr == "-p") {
            print_cfg = true;
        } else if (curr == "-c") {
            if (next.empty()) {
                cerr << "Error: missing argument to `-c'" << endl;
                print_tips = true;
                run = false;
                break;
            }

            ++i;
            if (!load(next)) {
                print_tips = true;
                run = false;
            }

        } else if (curr == "-s") {
            if (next.empty()) {
                cerr << "Error: missing argument to `-s'" << endl;
                print_tips = true;
                run = false;
                break;
            }

            ++i;
            if (!set(next)) {
                print_tips = true;
                run = false;
            }
        } else {
            cerr << "Error: invalid option `" << curr << "'" << endl;
            print_tips = true;
            run = false;
        }
    }

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
        << "OPTION:" << endl
        << "  -h, --help        display this help text and exit" << endl
        << "  -v, --version     display version and exit" << endl
        << "  -c FILE           specify config file, file type: JSON" << endl
        << "  -s KEY=VALUE      set config field" << endl
        << "  -p                display config" << endl
        << "  -n                not run" << endl
        << endl
        << "EXAMPLE:" << endl
        << "  " << proc_name << endl
        << "  " << proc_name << R"( -c somewhere/conf.json)" << endl
        << "  " << proc_name << R"( -c somewhere/conf.json -p)" << endl
        << "  " << proc_name << R"( -c somewhere/conf.json -p -n)" << endl
        << "  " << proc_name << R"( -s 'log.level=6' -s 'log.output="stdout"')" << endl
        << "  " << proc_name << R"( -s 'log={"level":5,"output":"tcp","tcp":{"ip":"192.168.0.20","port":50000}}')" << endl
        << "  " << proc_name << R"( -c somewhere/conf.json -s 'log.level=6' -s 'thread_pool.min_thread=2')" << endl
        << endl
        << "CONFIG:" << endl
        << R"(  type ")" << proc_name << R"( -p -n" to display default config.)" << endl
        << endl;
}

void Args::printVersion()
{
    int major, minor, rev, build;
    GetAppVersion(major, minor, rev, build);

    cout << "version : " << major << '.' << minor << '.' << rev << '_' << build << endl
         << "   buid : " << GetAppBuildTime() << endl;
}

bool Args::load(const std::string &config_filename)
{
    ifstream ifs(config_filename);
    if (!ifs) {
        cerr << "Error: can't open config file `" << config_filename << '\'' << endl;
        return false;
    }

    try {
        auto js_patch = Json::parse(ifs);
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
