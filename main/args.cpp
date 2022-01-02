#include "args.h"

#include <getopt.h>
#include <iostream>

#include <tbox/base/json.hpp>

namespace tbox::main {

using namespace std;

std::string GetAppDescribe();
std::string GetAppBuildTime();
void GetAppVersion(int &major, int &minor, int &rev, int &build);

Args::Args(Json &conf) :
    conf_(conf)
{ }

bool Args::parse(int argc, char **argv)
{
    const char *opt_str = "hvnpc:s:";
    const struct option opt_list[] = {
        { "help",    0, nullptr, 'h'},
        { "version", 0, nullptr, 'v'},
        { "config",  1, nullptr, 'c'},
        { "set",     1, nullptr, 's'},
        { "not_run", 0, nullptr, 'n'},
        { "print",   0, nullptr, 'p'},

        { 0, 0, nullptr, 0 },
    };

    bool run = true;    //!< 是否需要正常运行
    bool print = false; //!< 是否需要打印配置数据

    while (true) {
        int flag = getopt_long(argc, argv, opt_str, opt_list, nullptr);
        if (flag == -1)
            break;

        switch (flag) {
            case 'h':
            case '?':   //! 如果参数不合法
                printHelp(argv[0]);
                run = false;
                break;

            case 'v':
                printVersion();
                run = false;
                break;

            case 'n':
                run = false;
                break;

            case 'p':
                print = true;
                break;

            case 'c':
                loadConfig(optarg);
                break;

            case 's':
                set(optarg);
                break;
        }
    }

    if (print)
        cout << conf_.dump(2) << endl;

    return run;
}

void Args::printHelp(const char *proc_name)
{
    cout << "Usage: " << proc_name << " [OPTION]" << endl
        << GetAppDescribe() << endl << endl
        << "OPTION:" << endl
        << "  -h, --help            display this help text and exit" << endl
        << "  -v, --version         display version and exit" << endl
        << "  -p, --print           display config" << endl
        << "  -n, --not_run         not run and exit" << endl
        << "  -s, --set=TEXT        set config filed" << endl
        << "  -c, --config=FILE     specify config file, file context type: JSON" << endl
        << endl
        << "EXAMPLE:" << endl
        << "  " << proc_name << endl
        << "  " << proc_name << R"( -c somewhere/conf.json)" << endl
        << "  " << proc_name << R"( -c somewhere/conf.json -p)" << endl
        << "  " << proc_name << R"( -c somewhere/conf.json -pn)" << endl
        << "  " << proc_name << R"( -s 'log.level=6' -s 'log.output="stdout"')" << endl
        << "  " << proc_name << R"( -s 'log={"level":5,"output":"tcp","tcp":{"ip":"192.168.0.20","port":50000}}')" << endl
        << "  " << proc_name << R"( -c somewhere/conf.json -s 'log.level=6' -s 'thread_pool.min_thread=2')" << endl
        << endl
        << "CONFIG:" << endl
        << R"(  type ")" << proc_name << R"( -pn" to display default config.)" << endl
        << endl;
}

void Args::printVersion()
{
    int major, minor, rev, build;
    GetAppVersion(major, minor, rev, build);

    cout << "version : " << major << '.' << minor << '.' << rev << '_' << build << endl
         << "   buid : " << GetAppBuildTime() << endl;
}

void Args::loadConfig(const char *config_filename)
{
    cout << "loadConfig(" << config_filename << ")" << endl;
    //TODO:
}

void Args::set(const char *set_string)
{
    cout << "set(" << set_string << ")" << endl;
    //TODO:
}

}
