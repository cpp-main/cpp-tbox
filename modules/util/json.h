#ifndef TBOX_UTIL_JSON_H_20220908
#define TBOX_UTIL_JSON_H_20220908

#include <stdexcept>
#include <tbox/base/json_fwd.h>

namespace tbox {
namespace util {
namespace json {

//! true, false
bool GetField(const Json &js, const std::string &field_name, bool &field_value);
//! 0, 2, 1234
bool GetField(const Json &js, const std::string &field_name, unsigned int &field_value);
//! -12, -1, 0, 2, 1234
bool GetField(const Json &js, const std::string &field_name, int &field_value);
//! -12.34, 0.0, 1, 22, -3, 12.34
bool GetField(const Json &js, const std::string &field_name, double &field_value);
//! "hello world"
bool GetField(const Json &js, const std::string &field_name, std::string &field_value);

//! 检查是否存在字段，且是对象
bool HasObjectField(const Json &js, const std::string &field_name);
//! 检查是否存在字段，且是数组
bool HasArrayField(const Json &js, const std::string &field_name);
//! 检查是否存在字段，且为Boolean
bool HasBooleanField(const Json &js, const std::string &field_name);
//! 检查是否存在字段，且为小数或是整数
bool HasNumberField(const Json &js, const std::string &field_name);
//! 检查是否存在字段，且为小数
bool HasFloatField(const Json &js, const std::string &field_name);
//! 检查是否存在字段，且为整数
bool HasIntegerField(const Json &js, const std::string &field_name);
//! 检查是否存在字段，且为正整数
bool HasUnsignedField(const Json &js, const std::string &field_name);
//! 检查是否存在字段，且为字串
bool HasStringField(const Json &js, const std::string &field_name);

//! 打开文件失败异常
struct OpenFileError : public std::runtime_error {
    explicit OpenFileError(const std::string &filename) :
        std::runtime_error("open file " + filename + " fail") { }
};
//! 解析JSON文件失败异常
struct ParseJsonFileError : public std::runtime_error {
    explicit ParseJsonFileError(const std::string &filename, const std::string &detail) :
        std::runtime_error("parse json file " + filename + " fail, detail:" + detail) { }
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

}
}
}

#endif //TBOX_UTIL_JSON_H_20220908
