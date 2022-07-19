#include "log.h"
#include <iostream>
#include <sstream>
#include <tbox/base/json.hpp>
#include <tbox/terminal/session.h>
#include <tbox/util/fs.h>

namespace tbox {
namespace main {

using namespace terminal;
using namespace std;

Log::Log()
{
    stdout_.enable();
}

Log::~Log()
{
    cleanup();
}

void Log::fillDefaultConfig(Json &cfg) const
{
    cfg["log"] = R"(
 {
  "stdout": {
    "enable": true,
    "enable_color": true
  },
  "filelog": {
    "enable": true,
    "prefix": "sample",
    "path": "/tmp/tbox",
    "max_size": 1024,
    "enable_color": false 
  },
  "syslog": {
    "enable": false
  }
}
)"_json;
}

bool Log::initialize(const char *proc_name, Context &ctx, const Json &cfg)
{
    buildTerminalNodes(*ctx.terminal());

    if (cfg.contains("log")) {
        auto &js_log = cfg.at("log");
        if (js_log.contains("stdout")) {
            auto &js = js_log.at("stdout");
            if (js.contains("enable")) {
                auto &js_enable = js.at("enable");
                if (js_enable.is_boolean()) {
                    if (js_enable.get<bool>())
                        stdout_.enable();
                    else
                        stdout_.disable();
                } else {
                    cerr << "WARN: stdout.enable not boolean." << endl;
                }
            }

            if (js.contains("enable_color")) {
                auto &js_enable = js.at("enable_color");
                if (js_enable.is_boolean()) {
                    stdout_.enableColor(js_enable.get<bool>());
                } else {
                    cerr << "WARN: stdout.enable_color not boolean." << endl;
                }
            }
        }

        if (js_log.contains("syslog")) {
            auto &js = js_log.at("syslog");
            if (js.contains("enable")) {
                auto &js_enable = js.at("enable");
                if (js_enable.is_boolean()) {
                    if (js_enable.get<bool>())
                        syslog_.enable();
                    else
                        syslog_.disable();
                } else {
                    cerr << "WARN: syslog.enable not boolean." << endl;
                }
            }
        }

        do {
            if (!js_log.contains("filelog"))
                break;

            auto &js = js_log.at("filelog");
            if (!js.contains("path")) {
                cerr << "WARN: filelog.path not found." << endl;
                break;
            }

            auto &js_path = js.at("path");
            if (!js_path.is_string()) {
                cerr << "WARN: filelog.path not string." << endl;
                break;
            }

            std::string prefix = util::fs::Basename(proc_name); //! 默认前缀就是进程名
            if (js.contains("prefix")) {
                auto &js_prefix = js.at("prefix");
                if (js_prefix.is_string())
                    prefix = js_prefix.get<std::string>();
                else
                    cerr << "WARN: filelog.prefix not string, using proc_name instead." << endl;
            }

            if (!filelog_.initialize(js_path.get<std::string>(), prefix)) {
                cerr << "WARN: Init filelog fail." << endl;
                break;
            }

            if (js.contains("max_size")) {
                auto &js_max_size = js.at("max_size");
                if (js_max_size.is_number())
                    filelog_.setFileMaxSize(js_max_size.get<int>() * 1024);
                else
                    cerr << "WARN: filelog.max_size not number" << endl;
            }

            if (js.contains("enable")) {
                auto &js_enable = js.at("enable");
                if (js_enable.is_boolean()) {
                    if (js_enable.get<bool>())
                        filelog_.enable();
                    else
                        filelog_.disable();
                } else
                    cerr << "WARN: filelog.enable not boolean" << endl;
            }

            if (js.contains("enable_color")) {
                auto &js_enable = js.at("enable_color");
                if (js_enable.is_boolean()) {
                    filelog_.enableColor(js_enable.get<bool>());
                } else
                    cerr << "WARN: filelog.enable_color not boolean" << endl;
            }
        } while (false);
    }

    return true;
}

void Log::cleanup()
{
    filelog_.cleanup();
    filelog_.disable();
}

void Log::buildTerminalNodes(TerminalNodes &term)
{
    auto log_node = term.createDirNode("This is log directory");
    term.mountNode(term.rootNode(), log_node, "log");

    {
        auto func_node = term.createFuncNode(
            [this] (const Session &s, const Args &args) {
                std::stringstream ss;
                bool print_usage = true;
                if (args.size() >= 2) {
                    const auto &opt = args[1];
                    if (opt == "on") {
                        stdout_.enable();
                        ss << "on\r\n";
                        print_usage = false;
                    } else if (opt == "off") {
                        stdout_.disable();
                        ss << "off\r\n";
                        print_usage = false;
                    }
                }

                if (print_usage)
                    ss << "Usage: " << args[0] << " on|off\r\n";

                s.send(ss.str());
            }
        , "enable or disable stdout log channel");
        term.mountNode(log_node, func_node, "stdout_enable");
    }

    {
        auto func_node = term.createFuncNode(
            [this] (const Session &s, const Args &args) {
                std::stringstream ss;
                bool print_usage = true;
                if (args.size() >= 2) {
                    const auto &opt = args[1];
                    if (opt == "on") {
                        stdout_.enableColor(true);
                        ss << "on\r\n";
                        print_usage = false;
                    } else if (opt == "off") {
                        stdout_.enableColor(false);
                        ss << "off\r\n";
                        print_usage = false;
                    }
                }

                if (print_usage)
                    ss << "Usage: " << args[0] << " on|off\r\n";

                s.send(ss.str());
            }
        , "enable or disable stdout log color");
        term.mountNode(log_node, func_node, "stdout_color_enable");
    }

    {
        auto func_node = term.createFuncNode(
            [this] (const Session &s, const Args &args) {
                std::stringstream ss;
                bool print_usage = true;
                if (args.size() >= 2) {
                    const auto &opt = args[1];
                    if (opt == "on") {
                        syslog_.enable();
                        ss << "on\r\n";
                        print_usage = false;
                    } else if (opt == "off") {
                        syslog_.disable();
                        ss << "off\r\n";
                        print_usage = false;
                    }
                }

                if (print_usage)
                    ss << "Usage: " << args[0] << " on|off\r\n";

                s.send(ss.str());
            }
        , "enable or disable syslog log channel");
        term.mountNode(log_node, func_node, "syslog_enable");
    }

    {
        auto func_node = term.createFuncNode(
            [this] (const Session &s, const Args &args) {
                std::stringstream ss;
                bool print_usage = true;
                if (args.size() >= 2) {
                    const auto &opt = args[1];
                    if (opt == "on") {
                        filelog_.enable();
                        ss << "on\r\n";
                        print_usage = false;
                    } else if (opt == "off") {
                        filelog_.disable();
                        ss << "off\r\n";
                        print_usage = false;
                    }
                }

                if (print_usage)
                    ss << "Usage: " << args[0] << " on|off\r\n";

                s.send(ss.str());
            }
        , "enable or disable syslog log channel");
        term.mountNode(log_node, func_node, "filelog_enable");
    }

    {
        auto func_node = term.createFuncNode(
            [this] (const Session &s, const Args &args) {
                std::stringstream ss;
                bool print_usage = true;
                if (args.size() >= 2) {
                    const auto &opt = args[1];
                    if (opt == "on") {
                        filelog_.enableColor(true);
                        ss << "on\r\n";
                        print_usage = false;
                    } else if (opt == "off") {
                        filelog_.enableColor(false);
                        ss << "off\r\n";
                        print_usage = false;
                    }
                }

                if (print_usage)
                    ss << "Usage: " << args[0] << " on|off\r\n";

                s.send(ss.str());
            }
        , "enable or disable file log color");
        term.mountNode(log_node, func_node, "file_color_enable");
    }
}

}
}

