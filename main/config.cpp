#include "config.h"

#include <getopt.h>
#include <iostream>

#include <tbox/base/json.hpp>

namespace tbox::main {

using namespace std;

bool Config::parse(Json &conf, int argc, char **argv)
{
    const char *opt_str = "hvnpc:s:";
    const struct option opt_list[] = {
        { "help",    0, nullptr, 'h'},
        { "version", 0, nullptr, 'v'},
        { "config",  0, nullptr, 'c'},
        { "set",     0, nullptr, 's'},
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
                loadConfig(conf, optarg);
                break;

            case 's':
                set(conf, optarg);
                break;
        }
    }

    if (print)
        cout << conf.dump(2) << endl;

    return run;
}

void Config::printHelp(const char *proc_name)
{
    //TODO:
}

void Config::printVersion()
{
    //TODO:
}

void Config::loadConfig(Json &conf, const char *config_filename)
{
    //TODO:
}

void Config::set(Json &conf, const char *set_string)
{
    //TODO:
}

}
