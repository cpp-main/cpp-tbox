#include "json.h"
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

}
}
}
