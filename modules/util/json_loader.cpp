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
    includeByDescriptor(js_include.get<std::string>(), js_parent);
  } else if (js_include.is_array()) {
    for (auto &js_item : js_include) {
      if (js_item.is_string())
        includeByDescriptor(js_item.get<std::string>(), js_parent);
      else
        throw IncludeDescriptorTypeInvalid();
    }
  } else {
    throw IncludeDescriptorTypeInvalid();
  }
}

void JsonLoader::includeByDescriptor(const std::string &descriptor, Json &js) {
  LogTrace("%s", descriptor.c_str());
  std::vector<std::string> str_vec;
  string::Split(descriptor, "=>", str_vec);
  std::string filename = string::Strip(str_vec.at(0));

  auto js_load = load(filename);

  if (str_vec.size() >= 2) {
    auto keyname = string::Strip(str_vec.at(1));
    LogTrace("[%s]=%s", keyname.c_str(), js_load.dump().c_str());
    js[keyname] = std::move(js_load);
  } else {
    LogTrace("%s", js_load.dump().c_str());
    js.merge_patch(std::move(js_load));
  }
}

bool JsonLoader::checkRecursiveInclude(const std::string &filename) const {
  return std::find(files_.begin(), files_.end(), filename) != files_.end();
}

}
}
