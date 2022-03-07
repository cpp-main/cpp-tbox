#ifndef TBOX_BASE_JSON_H_20211227
#define TBOX_BASE_JSON_H_20211227

#include <nlohmann/json_fwd.hpp>

//! 如果找到不该文件，则从 github 下载：
//! https://raw.githubusercontent.com/nlohmann/json/v3.10.4/include/nlohmann/json_fwd.hpp
//! 放置到 /usr/local/include/nlohmann/ 目录下

namespace tbox {

using Json = nlohmann::json;
using OrderedJson = nlohmann::ordered_json;

}

#endif //TBOX_BASE_JSON_H_20211227
