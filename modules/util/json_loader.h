#ifndef TBOX_UTIL_JSON_LOADER_H_20221224
#define TBOX_UTIL_JSON_LOADER_H_20221224

#include <tbox/base/json_fwd.h>

namespace tbox {
namespace util {
namespace json {

//! include描述类型不合法，不是string
struct IncludeDescriptorTypeInvalid: public std::runtime_error {
    explicit IncludeDescriptorTypeInvalid() :
        std::runtime_error("include descriptor type error, it should be string") { }
};
//! 重复include同一个文件
struct DuplicateIncludeError : public std::runtime_error {
    explicit DuplicateIncludeError(const std::string &include_file) :
        std::runtime_error("duplicate include file:" + include_file) { }
};

class DeepLoader {
 public:
  Json load(const std::string &filename);

 protected:
  void traverse(Json &js);
  void handleInclude(const Json &js_include, Json &js_parent);
  void includeByDescriptor(const std::string &descriptor, Json &js);
  bool checkDuplicateInclude(const std::string &filename) const;

 private:
  std::vector<std::string> files_;
};

/// 加载JSON文件，并支持深度加载
/**
 * \param filename  JSON文件名
 * \return Json     解析所得的Json对象
 *
 * \throw OpenFileError
 *        ParseJsonFileError
 *        IncludeDescriptorTypeInvalid
 *        DuplicateIncludeError
 */
Json LoadDeeply(const std::string &filename);

}
}
}

#endif //TBOX_UTIL_JSON_LOADER_H_20221224
