#include "context_imp.h"

#include <cassert>
#include <sstream>
#include <iomanip>

#include <tbox/base/json.hpp>
#include <tbox/base/log.h>
#include <tbox/terminal/session.h>

namespace tbox::main {

ContextImp::ContextImp() :
    sp_loop_(event::Loop::New()),
    sp_thread_pool_(new eventx::ThreadPool(sp_loop_)),
    sp_timer_pool_(new eventx::TimerPool(sp_loop_)),
    sp_terminal_(new terminal::Terminal),
    sp_telnetd_(new terminal::Telnetd(sp_loop_, sp_terminal_)),
    sp_tcp_rpc_(new terminal::TcpRpc(sp_loop_, sp_terminal_))
{
    assert(sp_loop_ != nullptr);
    assert(sp_thread_pool_ != nullptr);
    assert(sp_timer_pool_ != nullptr);
    assert(sp_terminal_ != nullptr);
    assert(sp_telnetd_ != nullptr);
    assert(sp_tcp_rpc_ != nullptr);
}

ContextImp::~ContextImp()
{
    delete sp_tcp_rpc_;
    delete sp_telnetd_;
    delete sp_terminal_;
    delete sp_timer_pool_;
    delete sp_thread_pool_;
    delete sp_loop_;
}

void ContextImp::fillDefaultConfig(Json &cfg) const
{
    cfg["thread_pool"] = R"({"min":1, "max":0})"_json;
}

bool ContextImp::initialize(const Json &cfg)
{
    auto &js_thread_pool = cfg["thread_pool"];
    if (js_thread_pool.is_null()) {
        LogWarn("cfg.thread_pool not found");
        return false;
    }

    auto &js_thread_pool_min = js_thread_pool["min"];
    auto &js_thread_pool_max = js_thread_pool["max"];
    if (!js_thread_pool_min.is_number() || !js_thread_pool_max.is_number()) {
        LogWarn("in cfg.thread_pool, min or max is not number");
        return false;
    }

    if (!sp_thread_pool_->initialize(js_thread_pool_min.get<int>(), js_thread_pool_max.get<int>()))
        return false;

    if (cfg.contains("telnetd")) {
        auto &js_telnetd = cfg["telnetd"];
        if (js_telnetd.contains("bind")) {
            auto &js_telnetd_bind = js_telnetd["bind"];
            if (sp_telnetd_->initialize(js_telnetd_bind.get<std::string>()))
                telnetd_init_ok = true;
        }
    }
    if (cfg.contains("tcp_rpc")) {
        auto &js_tcp_rpc = cfg["tcp_rpc"];
        if (js_tcp_rpc.contains("bind")) {
            auto &js_tcp_rpc_bind = js_tcp_rpc["bind"];
            if (sp_tcp_rpc_->initialize(js_tcp_rpc_bind.get<std::string>()))
                tcp_rpc_init_ok = true;
        }
    }

    buildTerminalNodes();

    return true;
}

bool ContextImp::start()
{
    if (telnetd_init_ok)
        sp_telnetd_->start();

    if (tcp_rpc_init_ok)
        sp_tcp_rpc_->start();

    return true;
}

void ContextImp::stop()
{
    if (tcp_rpc_init_ok)
        sp_tcp_rpc_->stop();

    if (telnetd_init_ok)
        sp_telnetd_->stop();
}

void ContextImp::cleanup()
{
    sp_tcp_rpc_->cleanup();
    sp_telnetd_->cleanup();
    sp_timer_pool_->cleanup();
    sp_thread_pool_->cleanup();
}

void ContextImp::buildTerminalNodes()
{
    using namespace terminal;
    TerminalNodes *wp_nodes = sp_terminal_;

    auto ctx_node = wp_nodes->createDirNode("This is Context directory");
    wp_nodes->mountNode(wp_nodes->rootNode(), ctx_node, "context");

    auto loop_node = wp_nodes->createDirNode("This is Loop directory");
    wp_nodes->mountNode(ctx_node, loop_node, "loop");

    auto loop_stat_node = wp_nodes->createDirNode();
    wp_nodes->mountNode(loop_node, loop_stat_node, "stat");

    auto loop_stat_enable_node = wp_nodes->createFuncNode(
        [this] (const Session &s, const Args &args) {
            std::stringstream ss;
            if (args.size() >= 2) {
                const auto &opt = args[1];
                if (opt == "on") {
                    sp_loop_->setStatEnable(true);
                    ss << "stat on\r\n";
                } else if (opt == "off") {
                    sp_loop_->setStatEnable(false);
                    ss << "stat off\r\n";
                } else {
                    ss << "Usage: " << args[0] << " on|off\r\n";
                }
            } else {
                ss << (sp_loop_->isStatEnabled() ? "on" : "off") << "\r\n";
            }
            s.send(ss.str());
        }
    , "enable or disable Loop's stat function");
    wp_nodes->mountNode(loop_stat_node, loop_stat_enable_node, "enable");

    auto loop_stat_print_node = wp_nodes->createFuncNode(
        [this] (const Session &s, const Args &args) {
            std::stringstream ss;
            if (sp_loop_->isStatEnabled()) {
                auto stat = sp_loop_->getStat();
                ss  << "stat_time: " << stat.stat_time_us << " us\r\n"
                    << "time_cost: " << stat.time_cost_us << " us\r\n"
                    << "event_count: " << stat.event_count << "\r\n"
                    << "max_cost: " << stat.max_cost_us << " us\r\n";

                if (stat.event_count != 0)
                    ss << "avg_cost: " << stat.time_cost_us / stat.event_count << " us\r\n";

                if (stat.stat_time_us != 0)
                    ss << "cpu: " << std::setprecision(1) << stat.time_cost_us * 100.0 / stat.stat_time_us << " %\r\n";

            } else {
                ss << "stat not enabled\r\n";
            }
            s.send(ss.str());
        }
    , "print Loop's stat data");
    wp_nodes->mountNode(loop_stat_node, loop_stat_print_node, "print");

    auto loop_stat_reset_node = wp_nodes->createFuncNode(
        [this] (const Session &s, const Args &args) {
            std::stringstream ss;
            sp_loop_->resetStat();
            ss << "done\r\n";
            s.send(ss.str());
        }
    , "reset Loop's stat data");
    wp_nodes->mountNode(loop_stat_node, loop_stat_reset_node, "reset");
}

}
