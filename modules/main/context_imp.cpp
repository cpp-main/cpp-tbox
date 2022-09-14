#include "context_imp.h"

#include <cassert>
#include <sstream>
#include <iomanip>

#include <tbox/base/json.hpp>
#include <tbox/base/log.h>
#include <tbox/util/string.h>
#include <tbox/util/json.h>
#include <tbox/terminal/session.h>

namespace tbox {
namespace main {

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
    cfg["thread_pool"] = R"({"min":1, "max":5})"_json;
}

bool ContextImp::initialize(const Json &cfg)
{
    if (!util::json::HasObjectField(cfg, "thread_pool")) {
        LogWarn("cfg.thread_pool not found");
        return false;
    }
    auto &js_thread_pool = cfg["thread_pool"];

    int thread_pool_min = 0, thread_pool_max = 0;
    if (!util::json::GetField(js_thread_pool, "min", thread_pool_min) ||
        !util::json::GetField(js_thread_pool, "max", thread_pool_max)) {
        LogWarn("in cfg.thread_pool, min or max is not number");
        return false;
    }

    if (!sp_thread_pool_->initialize(thread_pool_min, thread_pool_max))
        return false;

    if (util::json::HasObjectField(cfg, "telnetd")) {
        auto &js_telnetd = cfg["telnetd"];
        std::string telnetd_bind;
        if (util::json::GetField(js_telnetd, "bind", telnetd_bind)) {
            if (sp_telnetd_->initialize(telnetd_bind))
                telnetd_init_ok = true;
        }
    }
    if (util::json::HasObjectField(cfg, "tcp_rpc")) {
        auto &js_tcp_rpc = cfg["tcp_rpc"];
        std::string tcp_rpc_bind;
        if (util::json::GetField(js_tcp_rpc, "bind", tcp_rpc_bind)) {
            if (sp_tcp_rpc_->initialize(tcp_rpc_bind))
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
    wp_nodes->mountNode(wp_nodes->rootNode(), ctx_node, "ctx");

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
                ss << sp_loop_->getStat();
            } else {
                ss << "stat not enabled" << std::endl;
            }
            std::string txt = ss.str();
            util::string::Replace(txt, "\n", "\r\n");
            s.send(txt);
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
}
