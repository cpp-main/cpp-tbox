#include "log.h"
#include <iostream>
#include <sstream>
#include <tbox/base/json.hpp>
#include <tbox/terminal/session.h>
#include <tbox/util/fs.h>
#include <tbox/util/json.h>

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
    "enable_color": true,
    "levels": {"":6}
  },
  "filelog": {
    "enable": false,
    "enable_color": false,
    "levels": {"":3},
    "prefix": "sample",
    "path": "/tmp/tbox",
    "max_size": 1024
  },
  "syslog": {
    "enable": false,
    "levels": {"":3}
  }
}
)"_json;
}

bool Log::initialize(const char *proc_name, Context &ctx, const Json &cfg)
{
    buildTerminalNodes(*ctx.terminal());

    if (util::json::HasObjectField(cfg, "log")) {
        auto &js_log = cfg.at("log");
        //! STDOUT
        if (util::json::HasObjectField(js_log, "stdout")) {
            auto &js_stdout = js_log.at("stdout");
            initChannel(js_stdout, stdout_);
        }

        //! SYSLOG
        if (util::json::HasObjectField(js_log, "syslog")) {
            auto &js_syslog = js_log.at("stdout");
            initChannel(js_syslog, syslog_);
        }

        //! FILELOG
        if (util::json::HasObjectField(js_log, "filelog")) {
            auto &js_filelog = js_log.at("filelog");
            do {
                std::string path;
                if (!util::json::GetField(js_filelog, "path", path)) {
                    cerr << "WARN: filelog.path not found or not string." << endl;
                    break;
                }

                initChannel(js_filelog, filelog_);

                std::string prefix = util::fs::Basename(proc_name); //! 默认前缀就是进程名
                util::json::GetField(js_filelog, "prefix", prefix);

                if (!filelog_.initialize(path, prefix)) {
                    cerr << "WARN: filelog init fail" << endl;
                    break;
                }

                unsigned int max_size = 1024;
                if (util::json::GetField(js_filelog, "max_size", max_size))
                    filelog_.setFileMaxSize(max_size * 1024);

            } while (false);
        }
    }
    return true;
}

void Log::cleanup()
{
    filelog_.cleanup();
    filelog_.disable();
}

void Log::initChannel(const Json &js, log::Channel &ch)
{
    bool enable = false;
    if (util::json::GetField(js, "enable", enable)) {
        if (enable)
            ch.enable();
        else
            ch.disable();
    }

    bool enable_color = false;
    if (util::json::GetField(js, "enable_color", enable_color))
        ch.enableColor(enable_color);

    if (util::json::HasObjectField(js, "levels")) {
        auto &js_levels = js.at("levels");
        for (auto &item : js_levels.items())
            ch.setLevel(item.value(), item.key());
    }
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

