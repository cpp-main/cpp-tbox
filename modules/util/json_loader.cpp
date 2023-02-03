#include "json_loader.h"
#include <fstream>

#include <tbox/base/json.hpp>
#include <tbox/base/log.h>
#include <tbox/util/json.h>
#include <tbox/util/string.h>
#include <tbox/util/fs.h>

namespace {
    const std::string kInclude = "__include__";
}

namespace tbox {
namespace util {
namespace js {

Json Loader::load(const std::string &filename) {
    LogTrace("%s", filename.c_str());
    if (checkRecursiveInclude(filename))
        throw RecursiveIncludeError(filename);

    files_.push_back(filename);
    Json js = Load(filename);
    traverse(js);
    files_.pop_back();
    return js;
}

void Loader::traverse(Json &js) {
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

void Loader::handleInclude(const Json &js_include, Json &js_parent) {
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

void Loader::includeByDescriptor(const std::string &descriptor, Json &js) {
    LogTrace("%s", descriptor.c_str());
    std::vector<std::string> str_vec;
    string::Split(descriptor, "=>", str_vec);
    std::string filename = string::Strip(str_vec.at(0));

    std::string real_filename;
    if (filename.front() != '/') {
        auto dir = fs::Dirname(files_.back());
        real_filename = dir + '/' + filename;
    } else {
        real_filename = filename;
    }
 
    auto js_load = load(real_filename);

    if (str_vec.size() >= 2) {
        auto keyname = string::Strip(str_vec.at(1));
        LogTrace("[%s]=%s", keyname.c_str(), js_load.dump().c_str());
        js[keyname] = std::move(js_load);
    } else {
        LogTrace("%s", js_load.dump().c_str());
        js.merge_patch(std::move(js_load));
    }
}

bool Loader::checkRecursiveInclude(const std::string &filename) const {
    return std::find(files_.begin(), files_.end(), filename) != files_.end();
}

Json Load(const std::string &filename)
{
    Json js;
    std::ifstream input_json_file(filename);
    if (input_json_file.is_open()) {
        try {
            input_json_file >> js;
        } catch (const std::exception &e) {
            throw ParseJsonFileError(filename, e.what());
        }
    } else {
        throw OpenFileError(filename);
    }

    return js;
}

Json LoadEx(const std::string &filename) {
    Loader loader;
    return loader.load(filename);
}

}
}
}
