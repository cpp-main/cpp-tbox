#include <tbox/base/assert.h>
#include <tbox/base/log.h>
#include <tbox/base/log_output.h>

int main() {
  LogOutput_Initialize("test");
  assert(12 > 0);
  assert(12 < 0);
  LogOutput_Cleanup();
  return 0;
}
