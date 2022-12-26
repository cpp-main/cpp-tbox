#ifndef TBOX_UTIL_JSON_LOADER_H_20221224
#define TBOX_UTIL_JSON_LOADER_H_20221224

#include <tbox/base/json_fwd.h>

namespace tbox {
namespace util {

class JsonLoader {
 public:
  struct OpenFileError : public std::runtime_error {
    explicit OpenFileError(const std::string &filename) :
      std::runtime_error("open file " + filename + " fail") { }
  };
  struct ParseJsonFileError : public std::runtime_error {
    explicit ParseJsonFileError(const std::string &filename, const std::string &detail) :
      std::runtime_error("parse json file " + filename + " fail, detail:" + detail) { }
  };
  struct IncludeTypeInvalid: public std::runtime_error {
    explicit IncludeTypeInvalid() :
      std::runtime_error("include filename type error, it should be string") { }
  };
  struct RecursiveIncludeError : public std::runtime_error {
    explicit RecursiveIncludeError(const std::string &include_file) :
      std::runtime_error("recursive include file:" + include_file) { }
  };

 public:
  explicit JsonLoader(const std::string &directory);
  Json load(const std::string &filename);

 protected:
  void traverse(Json &js);
  void handleInclude(const Json &js_include, Json &js_parent);
  void includeSubFile(const std::string &filename, Json &js);
  bool checkRecursiveInclude(const std::string &filename) const;

 private:
  std::string directory_;
  std::vector<std::string> files_;
};

}
}

#endif //TBOX_UTIL_JSON_LOADER_H_20221224
