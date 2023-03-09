#include "json.h"

#include <fstream>

#include <tbox/base/json.hpp>

namespace tbox {
namespace util {
namespace json {

bool Get(const Json &js, bool &field_value)
{
    if (!js.is_boolean())
        return false;
    field_value = js.get<bool>();
    return true;
}

bool Get(const Json &js, unsigned int &field_value) {
  if (!js.is_number_unsigned())
      return false;
  field_value = js.get<unsigned int>();
  return true;
}

bool Get(const Json &js, double &field_value)
{
    if (!js.is_number())
        return false;
    field_value = js.get<double>();
    return true;
}

bool Get(const Json &js, std::string &field_value)
{
    if (!js.is_string())
        return false;
    field_value = js.get<std::string>();
    return true;
}

bool Get(const Json &js,int &field_value)
{
    if (!js.is_number_integer())
        return false;
    field_value = js.get<int>();
    return true;
}

bool GetField(const Json &js, const std::string &field_name, bool &field_value)
{
    if (!js.contains(field_name))
        return false;
    return Get(js.at(field_name), field_value);
}

bool GetField(const Json &js, const std::string &field_name, unsigned int &field_value)
{
    if (!js.contains(field_name))
        return false;
    return Get(js.at(field_name), field_value);
}

bool GetField(const Json &js, const std::string &field_name, int &field_value)
{
    if (!js.contains(field_name))
        return false;
    return Get(js.at(field_name), field_value);
}

bool GetField(const Json &js, const std::string &field_name, double &field_value)
{
    if (!js.contains(field_name))
        return false;
    return Get(js.at(field_name), field_value);
}

bool GetField(const Json &js, const std::string &field_name, std::string &field_value)
{
    if (!js.contains(field_name))
        return false;
    return Get(js.at(field_name), field_value);
}

bool HasObjectField(const Json &js, const std::string &field_name)
{
    if (!js.contains(field_name))
        return false;
    auto &js_field = js.at(field_name);
    return js_field.is_object();
}

bool HasArrayField(const Json &js, const std::string &field_name)
{
    if (!js.contains(field_name))
        return false;
    auto &js_field = js.at(field_name);
    return js_field.is_array();
}

bool HasBooleanField(const Json &js, const std::string &field_name)
{
    if (!js.contains(field_name))
        return false;
    auto &js_field = js.at(field_name);
    return js_field.is_boolean();
}

bool HasNumberField(const Json &js, const std::string &field_name)
{
    if (!js.contains(field_name))
        return false;
    auto &js_field = js.at(field_name);
    return js_field.is_number();
}

bool HasFloatField(const Json &js, const std::string &field_name)
{
    if (!js.contains(field_name))
        return false;
    auto &js_field = js.at(field_name);
    return js_field.is_number_float();
}

bool HasIntegerField(const Json &js, const std::string &field_name)
{
    if (!js.contains(field_name))
        return false;
    auto &js_field = js.at(field_name);
    return js_field.is_number_integer();
}

bool HasUnsignedField(const Json &js, const std::string &field_name)
{
    if (!js.contains(field_name))
        return false;
    auto &js_field = js.at(field_name);
    return js_field.is_number_unsigned();
}

bool HasStringField(const Json &js, const std::string &field_name)
{
     if (!js.contains(field_name))
        return false;
    auto &js_field = js.at(field_name);
    return js_field.is_string();
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

}
}
}
