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
#include <tbox/base/log.h>
#include <tbox/base/json.hpp>
#include <tbox/base/catch_throw.h>
#include <tbox/terminal/session.h>
#include <tbox/terminal/helper.h>
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
  "max_size": 1024,
  "sync_enable": false
}
)"_json;
}

bool Trace::initialize(Context &ctx, const Json &cfg)
{
    initShell(*ctx.terminal());

    if (util::json::HasObjectField(cfg, "trace")) {
        auto &js_trace = cfg.at("trace");

        bool is_enable = false;
        bool is_sync_enable = false;
        std::string path_prefix;
        int max_size = 0;

        util::json::GetField(js_trace, "path_prefix", path_prefix);
        util::json::GetField(js_trace, "enable", is_enable);
        util::json::GetField(js_trace, "max_size", max_size);
        util::json::GetField(js_trace, "sync_enable", is_sync_enable);

        auto &sink = trace::Sink::GetInstance();

        if (max_size > 0)
            sink.setRecordFileMaxSize(1024 * max_size);

        if (is_sync_enable)
            sink.setFileSyncEnable(true);

        if (!path_prefix.empty())
            sink.setPathPrefix(path_prefix);

        if (is_enable)
            sink.enable();

        if (util::json::HasObjectField(js_trace, "filter")) {
            auto &js_filter = js_trace.at("filter");

            std::string filter_strategy;
            if (util::json::GetField(js_filter, "strategy", filter_strategy)) {
                if (filter_strategy == "permit") {
                    sink.setFilterStrategy(trace::Sink::FilterStrategy::kPermit);
                } else if (filter_strategy == "reject") {
                    sink.setFilterStrategy(trace::Sink::FilterStrategy::kReject);
                } else {
                    LogWarn("config 'trace.filter.strategy' field is invalid");
                }
            }

            if (util::json::HasArrayField(js_filter, "exempts")) {
                auto &js_exempts = js_filter.at("exempts");
                trace::Sink::ExemptSet exempt_set;
                for (auto &js_item : js_exempts) {
                    std::string module;
                    if (util::json::Get(js_item, module))
                        exempt_set.insert(module);
                    else
                        LogWarn("config 'trace.filter.exempts' item is invalid");
                }
                sink.setFilterExemptSet(exempt_set);
            }
        }
    }
    return true;
}

void Trace::initShell(TerminalNodes &term)
{
    auto trace_node = term.createDirNode("This is trace directory");
    term.mountNode(term.rootNode(), trace_node, "trace");

    {
        terminal::BooleanFuncNodeProfile profile;
        profile.get_func = [] { return trace::Sink::GetInstance().isEnabled(); };
        profile.set_func = \
            [] (bool is_enable) {
                if (is_enable)
                    return trace::Sink::GetInstance().enable();
                else
                    trace::Sink::GetInstance().disable();
                return true;
            };
        profile.usage = \
            "Usage: enable         # print current trace enable status\r\n"
            "       enable on|off  # enable or disable trace\r\n";
        profile.help = "Enable or disable trace, or check trace is enabled";

        terminal::AddFuncNode(term, trace_node, "set_enable", profile);
    }

    {
        terminal::BooleanFuncNodeProfile profile;
        profile.get_func = [] { return trace::Sink::GetInstance().isFileSyncEnabled(); };
        profile.set_func = \
            [] (bool is_enable) {
                trace::Sink::GetInstance().setFileSyncEnable(is_enable);
                return true;
            };
        profile.usage = \
            "Usage: file_sync_enable         # print file sync enable status\r\n"
            "       file_sync_enable on|off  # enable or disable file sync\r\n";
        profile.help = "Enable or disable file sync, or check file sync is enabled";
        terminal::AddFuncNode(term, trace_node, "set_file_sync_enable", profile);
    }

    {
        terminal::StringFuncNodeProfile profile;
        profile.set_func = [] (const std::string &text) { return trace::Sink::GetInstance().setPathPrefix(text); };
        profile.usage = \
            "Usage: set_path_prefix <path>\r\n"
            "Exp  : set_path_prefix /some/where/any_name\r\n";
        profile.help = "set trace path prefix";
        terminal::AddFuncNode(term, trace_node, "set_path_prefix", profile);
    }

    {
        terminal::IntegerFuncNodeProfile profile;
        profile.set_func = \
            [] (int max_size) {
                trace::Sink::GetInstance().setRecordFileMaxSize(max_size * 1024);
                return true;
            };
        profile.min_value = 1;
        profile.usage = \
            "Usage: set_max_size <size>  # Unit: KB, >=1\r\n"
            "Exp  : set_max_size 1024\r\n";
        profile.help = "set record file max size";
        terminal::AddFuncNode(term, trace_node, "set_record_max_size", profile);
    }

    {
        terminal::StringFuncNodeProfile profile;
        profile.get_func = [] () { return trace::Sink::GetInstance().getCurrRecordFilename(); };
        profile.help = "get current record file fullname";
        terminal::AddFuncNode(term, trace_node, "get_record_file", profile);
    }

    {
        terminal::StringFuncNodeProfile profile;
        profile.get_func = [] () { return trace::Sink::GetInstance().getDirPath(); };
        profile.help = "get current directory";
        terminal::AddFuncNode(term, trace_node, "get_dir_path", profile);
    }

    {
        auto filter_node = term.createDirNode();
        term.mountNode(trace_node, filter_node, "filter");

        {
            terminal::StringFuncNodeProfile profile;
            profile.get_func = [] () {
                auto strategy = trace::Sink::GetInstance().getFilterStrategy();
                if (strategy == trace::Sink::FilterStrategy::kPermit)
                    return "permit";
                else
                    return "reject";
            };
            profile.set_func = [] (const std::string &strategy) {
                if (strategy == "permit") {
                    trace::Sink::GetInstance().setFilterStrategy(trace::Sink::FilterStrategy::kPermit);
                    return true;

                } else if (strategy == "reject") {
                    trace::Sink::GetInstance().setFilterStrategy(trace::Sink::FilterStrategy::kReject);
                    return true;

                } else {
                    return false;
                }
            };

            profile.usage = \
                "Usage : strategy                 # print current filter strategy.\r\n"
                "        strategy permit|reject   # set filter strategy.\r\n";
            profile.help = "print or set filter strategy";
            terminal::AddFuncNode(term, filter_node, "strategy", profile);
        }

        {
            auto exempts_node = term.createDirNode();
            term.mountNode(filter_node, exempts_node, "exempts");

            {
                auto func_node = term.createFuncNode(
                    [] (const terminal::Session &s, const terminal::Args &) {
                        auto exempt_set = trace::Sink::GetInstance().getFilterExemptSet();
                        std::ostringstream oss;
                        for (auto &item : exempt_set)
                            oss << item << "\r\n";
                        oss << "\r\n";
                        s.send(oss.str());
                    }
                );
                term.mountNode(exempts_node, func_node, "print");
            }

            {
                terminal::StringFuncNodeProfile profile;
                profile.set_func = [] (const std::string &module) {
                    auto &sink = trace::Sink::GetInstance();
                    auto exempt_set = sink.getFilterExemptSet();
                    exempt_set.insert(module);
                    sink.setFilterExemptSet(exempt_set);
                    return true;
                };
                profile.usage = "Usage : add <module>\r\n";
                profile.help = "add module in exempts";
                terminal::AddFuncNode(term, exempts_node, "add", profile);
            }

            {
                terminal::StringFuncNodeProfile profile;
                profile.set_func = [] (const std::string &module) {
                    auto &sink = trace::Sink::GetInstance();
                    auto exempt_set = sink.getFilterExemptSet();
                    exempt_set.erase(module);
                    sink.setFilterExemptSet(exempt_set);
                    return true;
                };
                profile.usage = "Usage : remove <module>\r\n";
                profile.help = "remove module from exempts";
                terminal::AddFuncNode(term, exempts_node, "remove", profile);
            }

            terminal::AddFuncNode(term, exempts_node, "reset",
                [] {
                    auto &sink = trace::Sink::GetInstance();
                    sink.setFilterExemptSet(trace::Sink::ExemptSet());
                    return true;
                }
            );
        }
    }
}

}
}

