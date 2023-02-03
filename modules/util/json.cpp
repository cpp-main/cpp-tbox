#include "json.h"

#include <fstream>

#include <tbox/base/json.hpp>

namespace tbox {
namespace util {
namespace json {

bool GetField(const Json &js, const std::string &field_name, bool &field_value)
{
    if (!js.contains(field_name))
        return false;
    auto &js_field = js.at(field_name);
    if (!js_field.is_boolean())
        return false;
    field_value = js_field.get<bool>();
    return true;
}

bool GetField(const Json &js, const std::string &field_name, unsigned int &field_value)
{
    if (!js.contains(field_name))
        return false;
    auto &js_field = js.at(field_name);
    if (!js_field.is_number_unsigned())
        return false;
    field_value = js_field.get<unsigned int>();
    return true;
}

bool GetField(const Json &js, const std::string &field_name, int &field_value)
{
    if (!js.contains(field_name))
        return false;
    auto &js_field = js.at(field_name);
    if (!js_field.is_number_integer())
        return false;
    field_value = js_field.get<int>();
    return true;
}

bool GetField(const Json &js, const std::string &field_name, double &field_value)
{
    if (!js.contains(field_name))
        return false;
    auto &js_field = js.at(field_name);
    if (!js_field.is_number())
        return false;
    field_value = js_field.get<double>();
    return true;
}

bool GetField(const Json &js, const std::string &field_name, std::string &field_value)
{
    if (!js.contains(field_name))
        return false;
    auto &js_field = js.at(field_name);
    if (!js_field.is_string())
        return false;
    field_value = js_field.get<std::string>();
    return true;
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

}
}
}
