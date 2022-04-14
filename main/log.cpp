#include "log.h"
#include <tbox/base/json.hpp>
#include <tbox/terminal/session.h>

namespace tbox {
namespace main {

using namespace terminal;

Log::Log()
{
    stdout_.enable();
}

Log::~Log()
{
    cleanup();
}

bool Log::initialize(Context &ctx, const Json &cfg)
{
    buildTerminalNodes(*ctx.terminal());

#if 0
    if (cfg.contains("log")) {
        auto &js_log = cfg.at("log");
    }
#endif

    return true;
}

void Log::cleanup()
{
    filelog_.cleanup();
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
}

}
}
