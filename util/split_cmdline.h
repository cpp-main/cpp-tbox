#ifndef TBOX_UTIL_SPLIT_CMDLINE_H_20220207
#define TBOX_UTIL_SPLIT_CMDLINE_H_20220207

#include <string>
#include <vector>

namespace tbox {
namespace util {

bool SplitCmdline(const std::string &cmd, std::vector<std::string> &args);

}
}

#endif //TBOX_UTIL_SPLIT_CMDLINE_H_20220207
