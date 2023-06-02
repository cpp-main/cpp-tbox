#ifndef TBOX_MAIN_LOG_H_20220414
#define TBOX_MAIN_LOG_H_20220414

#include <base/json_fwd.h>

#include <log/stdout_channel.h>
#include <log/syslog_channel.h>
#include <log/filelog_channel.h>

#include <terminal/terminal_nodes.h>

#include "context.h"

namespace tbox {
namespace main {

class Log {
  public:
    Log();

  public:
    void fillDefaultConfig(Json &cfg) const;
    bool initialize(const char *proc_name, Context &ctx, const Json &cfg);
    void cleanup();

  protected:
    void initChannel(const Json &js, log::Channel &ch);
    void initShell(terminal::TerminalNodes &term);
    void initShellForChannel(log::Channel &log_ch, terminal::TerminalNodes &term, terminal::NodeToken dir_node);
    void initShellForFilelogChannel(terminal::TerminalNodes &term, terminal::NodeToken dir_node);

  private:
    log::StdoutChannel stdout_;
    log::SyslogChannel syslog_;
    log::FilelogChannel filelog_;
};

}
}

#endif //TBOX_MAIN_LOG_H_20220414
