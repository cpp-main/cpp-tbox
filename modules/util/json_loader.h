#ifndef TBOX_UTIL_JSON_LOADER_H_20221224
#define TBOX_UTIL_JSON_LOADER_H_20221224

#include <tbox/base/json_fwd.h>

namespace tbox {
namespace util {
namespace js {

struct OpenFileError : public std::runtime_error {
    explicit OpenFileError(const std::string &filename) :
        std::runtime_error("open file " + filename + " fail") { }
};
struct ParseJsonFileError : public std::runtime_error {
    explicit ParseJsonFileError(const std::string &filename, const std::string &detail) :
        std::runtime_error("parse json file " + filename + " fail, detail:" + detail) { }
};
struct IncludeDescriptorTypeInvalid: public std::runtime_error {
    explicit IncludeDescriptorTypeInvalid() :
        std::runtime_error("include descriptor type error, it should be string") { }
};
struct RecursiveIncludeError : public std::runtime_error {
    explicit RecursiveIncludeError(const std::string &include_file) :
        std::runtime_error("recursive include file:" + include_file) { }
};

class Loader {
 public:
  Json load(const std::string &filename);

 protected:
  void traverse(Json &js);
  void handleInclude(const Json &js_include, Json &js_parent);
  void includeByDescriptor(const std::string &descriptor, Json &js);
  bool checkRecursiveInclude(const std::string &filename) const;

 private:
  std::vector<std::string> files_;
};

/// 加载JSON文件
/**
 * \param filename  JSON文件名
 * \return Json     解析所得的Json对象
 *
 * \throw OpenFileError
 *        ParseJsonFileError
 */
Json Load(const std::string &filename);

/// 加载JSON文件，支持 "__include__" 关键字
/**
 * \param filename  JSON文件名
 * \return Json     解析所得的Json对象
 *
 * \throw OpenFileError,
 *        ParseJsonFileError,
 *        IncludeDescriptorTypeInvalid,
 *        RecursiveIncludeError
 */
Json LoadEx(const std::string &filename);

}
}
}

#endif //TBOX_UTIL_JSON_LOADER_H_20221224
