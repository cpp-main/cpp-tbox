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
    "levels": {"":4},
    "max_size": 1024
  },
  "syslog": {
    "enable": false,
    "levels": {"":4}
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
            auto &js_syslog = js_log.at("syslog");
            initChannel(js_syslog, syslog_);
        }

        //! FILELOG
        if (util::json::HasObjectField(js_log, "filelog")) {
            auto &js_filelog = js_log.at("filelog");

            std::string path;
            if (util::json::GetField(js_filelog, "path", path))
                filelog_.setFilePath(path);

            std::string prefix = util::fs::Basename(proc_name);
            util::json::GetField(js_filelog, "prefix", prefix);
            filelog_.setFilePrefix(prefix);

            unsigned int max_size = 0;
            if (util::json::GetField(js_filelog, "max_size", max_size))
                filelog_.setFileMaxSize(max_size * 1024);

            initChannel(js_filelog, filelog_);
        }
    }
    return true;
}

void Log::cleanup()
{
    filelog_.disable();
    syslog_.disable();
    stdout_.disable();

    filelog_.cleanup();
    syslog_.cleanup();
    stdout_.cleanup();
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
        auto dir_node = term.createDirNode();
        term.mountNode(log_node, dir_node, "stdout");
        buildTerminalNodesForChannel(term, stdout_, dir_node);
    }
    {
        auto dir_node = term.createDirNode();
        term.mountNode(log_node, dir_node, "syslog");
        buildTerminalNodesForChannel(term, syslog_, dir_node);
    }
    {
        auto dir_node = term.createDirNode();
        term.mountNode(log_node, dir_node, "filelog");
        buildTerminalNodesForChannel(term, filelog_, dir_node);
    }
}

void Log::buildTerminalNodesForChannel(terminal::TerminalNodes &term, log::Channel &log_ch, terminal::NodeToken dir_node)
{
    {
        auto func_node = term.createFuncNode(
            [this, &log_ch] (const Session &s, const Args &args) {
                std::stringstream ss;
                bool print_usage = true;
                if (args.size() >= 2) {
                    const auto &opt = args[1];
                    if (opt == "on") {
                        log_ch.enable();
                        ss << "on\r\n";
                        print_usage = false;
                    } else if (opt == "off") {
                        log_ch.disable();
                        ss << "off\r\n";
                        print_usage = false;
                    }
                }

                if (print_usage)
                    ss << "Usage: " << args[0] << " on|off\r\n";

                s.send(ss.str());
            }
        , "enable or disable");
        term.mountNode(dir_node, func_node, "enable");
    }

    {
        auto func_node = term.createFuncNode(
            [this, &log_ch] (const Session &s, const Args &args) {
                std::stringstream ss;
                bool print_usage = true;
                if (args.size() >= 2) {
                    const auto &opt = args[1];
                    if (opt == "on") {
                        log_ch.enableColor(true);
                        ss << "on\r\n";
                        print_usage = false;
                    } else if (opt == "off") {
                        log_ch.enableColor(false);
                        ss << "off\r\n";
                        print_usage = false;
                    }
                }

                if (print_usage)
                    ss << "Usage: " << args[0] << " on|off\r\n";

                s.send(ss.str());
            }
        , "enable or disable color");
        term.mountNode(dir_node, func_node, "color_enable");
    }

    {
        auto func_node = term.createFuncNode(
            [this, &log_ch] (const Session &s, const Args &args) {
                std::stringstream ss;
                bool print_usage = true;
                if (args.size() >= 3) {
                    do {
                        auto &module_id = args[1];
                        int level = 0;
                        try {
                            level = std::stoi(args[2]);
                        } catch (const std::exception &e) {
                            ss << "level must be number\r\n";
                            break;
                        }
                        if (level < 0 || level > LOG_LEVEL_TRACE) {
                            ss << "level range: [0-" << LOG_LEVEL_TRACE << "]\r\n";
                            break;
                        }

                        log_ch.setLevel(level, module_id);
                        ss << "done\r\n";
                        print_usage = false;

                    } while (false);
                }

                if (print_usage)
                    ss << "Usage: " << args[0] << " <module_id:string> <level:0-6>\r\n"
                       << "LEVEL\r\n"
                       << " 0: Fatal\r\n"
                       << " 1: Error\r\n"
                       << " 2: Warn\r\n"
                       << " 3: Notice\r\n"
                       << " 4: Info\r\n"
                       << " 5: Debug\r\n"
                       << " 6: Trace\r\n"
                       ;

                s.send(ss.str());
            }
        , "set log level");
        term.mountNode(dir_node, func_node, "set_level");
    }
}

}
}

