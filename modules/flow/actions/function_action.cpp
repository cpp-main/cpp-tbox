#include "function_action.h"
#include <tbox/base/assert.h>
#include <tbox/event/loop.h>

namespace tbox {
namespace flow {

FunctionAction::FunctionAction(event::Loop &loop, const Func &func) :
  Action(loop, "Function"), func_(func)
{
  TBOX_ASSERT(func != nullptr);
}

bool FunctionAction::onStart() {
  loop_.runNext([this] { finish(func_()); });
  return true;
}

}
}
