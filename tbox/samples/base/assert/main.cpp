#include <tbox/base/assert.h>
#include <tbox/base/log.h>
#include <tbox/base/log_output.h>

int main() {
  LogOutput_Initialize();
  TBOX_ASSERT(12 > 0);
  TBOX_ASSERT(12 < 0);
  LogOutput_Cleanup();
  return 0;
}
