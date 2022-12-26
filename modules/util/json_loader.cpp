#include "json_loader.h"
#include <fstream>

#include <tbox/base/json.hpp>
#include <tbox/base/log.h>
#include <tbox/util/json.h>
#include <tbox/util/string.h>

namespace {
  const std::string kInclude = "__include__";
}

namespace tbox {
namespace util {

JsonLoader::JsonLoader(const std::string &directory) :
  directory_(directory)
{
  if (directory_.back() != '/')
    directory_.push_back('/');
}

Json JsonLoader::load(const std::string &filename) {
  LogTrace("%s", filename.c_str());
  if (checkRecursiveInclude(filename))
    throw RecursiveIncludeError(filename);

  files_.push_back(filename);
  auto full_filename = directory_ + filename;
  Json js;
  std::ifstream input_json_file(full_filename);
  if (input_json_file.is_open()) {
    try {
      input_json_file >> js;
    } catch (const std::exception &e) {
      throw ParseJsonFileError(full_filename, e.what());
    }
  } else {
    throw OpenFileError(full_filename);
  }
  
  traverse(js);
  files_.pop_back();
  return js;
}

void JsonLoader::traverse(Json &js) {
  if (js.is_object()) {
    Json js_patch;
    for (auto &item : js.items()) {
      auto &js_value = item.value();
      if (item.key() == kInclude) {
        handleInclude(js_value, js_patch);
      } else {
        traverse(js_value);
      }
    }
    js.erase(kInclude);

    if (!js_patch.is_null())
      js.merge_patch(js_patch);

  } else if (js.is_array()) {
    for (auto &js_item : js) {
      traverse(js_item);
    }
  }
}

void JsonLoader::handleInclude(const Json &js_include, Json &js_parent) {
  if (js_include.is_string()) {
    includeSubFile(js_include.get<std::string>(), js_parent);
  } else if (js_include.is_array()) {
    for (auto &js_item : js_include) {
      if (js_item.is_string())
        includeSubFile(js_item.get<std::string>(), js_parent);
      else
        throw IncludeTypeInvalid();
    }
  } else {
    throw IncludeTypeInvalid();
  }
}

void JsonLoader::includeSubFile(const std::string &filename, Json &js) {
  LogTrace("%s", filename.c_str());
  auto js_load = load(filename);
  js.merge_patch(std::move(js_load));
}

bool JsonLoader::checkRecursiveInclude(const std::string &filename) const {
  return std::find(files_.begin(), files_.end(), filename) != files_.end();
}

}
}
