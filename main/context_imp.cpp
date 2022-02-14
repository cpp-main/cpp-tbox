#include "context_imp.h"

#include <cassert>
#include <sstream>

#include <tbox/base/json.hpp>
#include <tbox/base/log.h>
#include <tbox/terminal/session.h>

namespace tbox::main {

ContextImp::ContextImp() :
    sp_loop_(event::Loop::New()),
    sp_thread_pool_(new eventx::ThreadPool(sp_loop_)),
    sp_timer_pool_(new eventx::TimerPool(sp_loop_)),
    sp_terminal_(new terminal::Terminal),
    sp_telnetd_(new terminal::Telnetd(sp_loop_, sp_terminal_))
{
    assert(sp_loop_ != nullptr);
    assert(sp_thread_pool_ != nullptr);
    assert(sp_timer_pool_ != nullptr);
    assert(sp_terminal_ != nullptr);
    assert(sp_telnetd_ != nullptr);
}

ContextImp::~ContextImp()
{
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

    buildTerminalNodes();

    return true;
}

bool ContextImp::start()
{
    if (telnetd_init_ok)
        sp_telnetd_->start();
    return true;
}

void ContextImp::stop()
{
    if (telnetd_init_ok)
        sp_telnetd_->stop();
}

void ContextImp::cleanup()
{
    sp_telnetd_->cleanup();
    sp_timer_pool_->cleanup();
    sp_thread_pool_->cleanup();
}

void ContextImp::buildTerminalNodes()
{
    using namespace terminal;

    auto ctx_node = sp_terminal_->createDirNode("This is Context directory");
    sp_terminal_->mountNode(sp_terminal_->rootNode(), ctx_node, "context");

    auto loop_node = sp_terminal_->createDirNode("This is Loop directory");
    sp_terminal_->mountNode(ctx_node, loop_node, "loop");

    auto loop_stat_enable = sp_terminal_->createFuncNode(
        [this] (const Session &s, const Args &args) {
            std::stringstream ss;
            if (args.size() == 2) {
                const auto &opt = args[1];
                if (opt == "on") {
                    sp_loop_->setStatEnable(true);
                    ss << "stat on\r\n";
                } else if (opt == "off") {
                    sp_loop_->setStatEnable(false);
                    ss << "stat off\r\n";
                } else {
                    ss << "Unknown option.\r\n";
                }
            } else {
                ss << "Usage: " << args[0] << " on|off\r\n";
            }
            s.send(ss.str());
        }
    , "enable or disable Loop's stat function");
    sp_terminal_->mountNode(loop_node, loop_stat_enable, "stat_enable");

    auto loop_stat_print = sp_terminal_->createFuncNode(
        [this] (const Session &s, const Args &args) {
            std::stringstream ss;
            auto stat = sp_loop_->getStat();
            ss  << "stat_time: " << stat.stat_time_us << "us\r\n"
                << "time_cost: " << stat.time_cost_us << "us\r\n"
                << "max_cost: " << stat.max_cost_us << "us\r\n"
                << "event_count: " << stat.event_count << "\r\n"
                ;
            s.send(ss.str());
        }
    , "print Loop's stat data");
    sp_terminal_->mountNode(loop_node, loop_stat_print, "stat_print");

    auto loop_stat_reset = sp_terminal_->createFuncNode(
        [this] (const Session &s, const Args &args) {
            std::stringstream ss;
            sp_loop_->resetStat();
            ss << "done\r\n";
            s.send(ss.str());
        }
    , "reset Loop's stat data");
    sp_terminal_->mountNode(loop_node, loop_stat_reset, "stat_reset");
}

}
