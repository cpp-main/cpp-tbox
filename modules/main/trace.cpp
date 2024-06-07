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
 * Copyright (c) 2024 Hevake and contributors, all rights reserved.
 *
 * This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
 * Use of this source code is governed by MIT license that can be found
 * in the LICENSE file in the root of the source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */
#include "trace.h"
#include <iostream>
#include <sstream>
#include <tbox/base/json.hpp>
#include <tbox/base/catch_throw.h>
#include <tbox/terminal/session.h>
#include <tbox/util/json.h>
#include <tbox/trace/sink.h>

namespace tbox {
namespace main {

using namespace terminal;
using namespace std;

void Trace::fillDefaultConfig(Json &cfg) const
{
    cfg["trace"] = R"(
{
  "enable": false,
  "path_prefix": "/tmp/trace/unknown",
  "max_size": 1024,
  "enable_sync": false
}
)"_json;
}

bool Trace::initialize(Context &ctx, const Json &cfg)
{
    initShell(*ctx.terminal());

    if (util::json::HasObjectField(cfg, "trace")) {
        auto &js_trace = cfg.at("trace");

        bool is_enable = false;
        bool is_enable_sync = false;
        std::string path_prefix;
        int max_size = 0;

        util::json::GetField(js_trace, "path_prefix", path_prefix);
        util::json::GetField(js_trace, "enable", is_enable);
        util::json::GetField(js_trace, "max_size", max_size);
        util::json::GetField(js_trace, "enable_sync", is_enable_sync);

        auto &sink = trace::Sink::GetInstance();

        if (!path_prefix.empty()) {
            sink.setPathPrefix(path_prefix);
            if (is_enable)
                sink.enable();
        }

        if (max_size > 0)
            sink.setRecordFileMaxSize(1024 * max_size);

        if (is_enable_sync)
            sink.setFileSyncEnable(true);
    }
    return true;
}

void Trace::initShell(TerminalNodes &term)
{
    auto trace_node = term.createDirNode("This is log directory");
    term.mountNode(term.rootNode(), trace_node, "trace");

    {
        auto func_node = term.createFuncNode(
            [] (const Session &s, const Args &args) {
                std::ostringstream oss;
                bool print_usage = true;
                if (args.size() >= 2) {
                    const auto &opt = args[1];
                    auto &sink = trace::Sink::GetInstance();
                    if (opt == "on") {
                        sink.enable();
                        oss << "on\r\n";
                        print_usage = false;

                    } else if (opt == "off") {
                        sink.disable();
                        oss << "off\r\n";
                        print_usage = false;
                    }
                }

                if (print_usage)
                    oss << "Usage: " << args[0] << " on|off\r\n";

                s.send(oss.str());
            }
        , "enable or disable");
        term.mountNode(trace_node, func_node, "set_enable");
    }

    {
        auto func_node = term.createFuncNode(
            [] (const Session &s, const Args &args) {
                std::ostringstream oss;
                bool print_usage = true;
                if (args.size() >= 2) {
                    const auto &opt = args[1];
                    auto &sink = trace::Sink::GetInstance();
                    if (opt == "on") {
                        sink.setFileSyncEnable(true);
                        oss << "on\r\n";
                        print_usage = false;

                    } else if (opt == "off") {
                        sink.setFileSyncEnable(false);
                        oss << "off\r\n";
                        print_usage = false;
                    }
                }

                if (print_usage)
                    oss << "Usage: " << args[0] << " on|off\r\n";

                s.send(oss.str());
            }
        , "enable or disable file sync");
        term.mountNode(trace_node, func_node, "set_enable_sync");
    }

    {
        auto func_node = term.createFuncNode(
            [this] (const Session &s, const Args &args) {
                std::ostringstream oss;
                bool print_usage = true;
                if (args.size() >= 2) {
                    trace::Sink::GetInstance().setPathPrefix(args.at(1));
                    print_usage = false;
                    oss << "done\r\n";
                }

                if (print_usage) {
                    oss << "Usage: " << args[0] << " <path>\r\n"
                        << "Exp  : " << args[0] << " /some/where/any_name\r\n";
                }

                s.send(oss.str());
            }
        , "set trace path prefix");
        term.mountNode(trace_node, func_node, "set_path_prefix");
    }

    {
        auto func_node = term.createFuncNode(
            [this] (const Session &s, const Args &args) {
                std::ostringstream oss;
                bool print_usage = true;
                if (args.size() >= 2) {
                    size_t max_size;
                    bool is_throw = CatchThrowQuietly([&] { max_size = std::stoul(args.at(1)); });
                    if (!is_throw) {
                        trace::Sink::GetInstance().setRecordFileMaxSize(max_size * 1024);
                        print_usage = false;
                        oss << "done, max_size: " << max_size << " KB\r\n";
                    } else {
                        oss << "size must be number\r\n";
                    }
                }

                if (print_usage) {
                    oss << "Usage: " << args[0] << " <size>\r\n"
                        << "Exp  : " << args[0] << " 1024\r\n";
                }

                s.send(oss.str());
            }
        , "set record file max size");
        term.mountNode(trace_node, func_node, "set_max_size");
    }
}

}
}

