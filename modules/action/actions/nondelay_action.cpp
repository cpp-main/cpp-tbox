#include "nondelay_action.h"
#include <tbox/base/assert.h>

namespace tbox {
namespace action {

NondelayAction::NondelayAction(event::Loop &loop, const std::string &id, const Func &func) :
  Action(loop, id), func_(func)
{
  assert(func != nullptr);
}

bool NondelayAction::onStart() {
  finish(func_());
  return true;
}

}
}
