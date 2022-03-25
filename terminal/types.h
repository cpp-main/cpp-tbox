#ifndef TBOX_TERMINAL_TYPES_H_20220128
#define TBOX_TERMINAL_TYPES_H_20220128

#include <vector>
#include <string>
#include <functional>
#include <tbox/base/cabinet_token.h>

namespace tbox {
namespace terminal {

class Session;

using SessionToken = cabinet::Token;
using NodeToken    = cabinet::Token;

using Args = std::vector<std::string>;
using Func = std::function<void (const Session &s, const Args &)>;

}
}

#endif //TBOX_TERMINAL_TYPES_H_20220128
