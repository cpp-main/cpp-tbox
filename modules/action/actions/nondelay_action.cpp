#include "nondelay_action.h"
#include <tbox/base/assert.h>
#include <tbox/event/loop.h>

namespace tbox {
namespace action {

NondelayAction::NondelayAction(event::Loop &loop, const std::string &id, const Func &func) :
  Action(loop, id), func_(func)
{
  assert(func != nullptr);
}

bool NondelayAction::onStart() {
  loop_.run([this] { finish(func_()); });
  return true;
}

}
}
