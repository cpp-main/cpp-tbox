#include "function_action.h"
#include <tbox/base/assert.h>
#include <tbox/event/loop.h>

namespace tbox {
namespace flow {

FunctionAction::FunctionAction(event::Loop &loop) :
  Action(loop, "Function")
{ }

bool FunctionAction::onStart() {
  loop_.runNext([this] { finish(func_()); }, "FunctionAction::onStart");
  return true;
}

}
}
