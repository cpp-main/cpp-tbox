#include "print_log.h"
#include <tbox/base/log.h>
#include "../context.h"

namespace tbox {
namespace http {
namespace server {

PrintLog::PrintLog(int level) :
    level_(level)
{ }

void PrintLog::handle(ContextSptr sp_ctx, const NextFunc &next)
{
    LogPrintf(level_, "REQ: [%s]", sp_ctx->req().toString().c_str());
    next();
    LogPrintf(level_, "RES: [%s]", sp_ctx->res().toString().c_str());
}

}
}
}
