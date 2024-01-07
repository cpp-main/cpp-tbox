#ifndef TBOX_BASE_FUNC_TYPES_H_20240107
#define TBOX_BASE_FUNC_TYPES_H_20240107

#include <functional>

//! 定义最常用的std::function类型

namespace tbox {

using VoidFunc = std::function<void()>;
using BoolFunc = std::function<bool()>;

}

#endif //TBOX_BASE_FUNC_TYPES_H_20240107
