#pragma once

#include "plugin_api.h"

#include <cstring>

namespace calc::plugin
{
namespace detail
{

inline const char* common_validation_error(const CalcPluginApi& api) noexcept
{
    if (api.abi_version != CALC_PLUGIN_ABI_VERSION || api.struct_size != sizeof(CalcPluginApi))
    {
        return "Incompatible plugin ABI";
    }
    if (api.invoke == nullptr || api.argument_count == 0 ||
        api.argument_count > CALC_PLUGIN_MAX_ARGUMENTS)
    {
        return "Invalid callback or argument count";
    }
    if (api.name[0] == '\0' || std::memchr(api.name, '\0', sizeof(api.name)) == nullptr)
    {
        return "Missing or unterminated operation name";
    }
    return nullptr;
}

inline bool is_identifier_start(char character) noexcept
{
    return (character >= 'a' && character <= 'z') || (character >= 'A' && character <= 'Z') ||
           character == '_';
}

inline bool valid_function_identifier(const char* name) noexcept
{
    if (!is_identifier_start(name[0]))
    {
        return false;
    }
    for (uint32_t index = 1; name[index] != '\0'; ++index)
    {
        const char character = name[index];
        const bool digit     = character >= '0' && character <= '9';
        if (!is_identifier_start(character) && !digit)
        {
            return false;
        }
    }
    return true;
}

inline const char* function_validation_error(const CalcPluginApi& api) noexcept
{
    if (api.precedence != 0 || api.associativity != CALC_PLUGIN_ASSOC_NONE)
    {
        return "Named functions have no precedence or associativity";
    }
    if (!valid_function_identifier(api.name))
    {
        return "Invalid function identifier";
    }
    return nullptr;
}

inline const char* infix_validation_error(const CalcPluginApi& api) noexcept
{
    if (api.name[1] != '\0' || std::strchr("^%&|~!@?:", api.name[0]) == nullptr)
    {
        return "Invalid or reserved operator symbol";
    }
    if (api.argument_count != 2 || api.precedence == 0 || api.precedence > 100 ||
        (api.associativity != CALC_PLUGIN_ASSOC_LEFT &&
         api.associativity != CALC_PLUGIN_ASSOC_RIGHT))
    {
        return "Invalid binary operator parsing rules";
    }
    return nullptr;
}

} // namespace detail

inline const char* validation_error(const CalcPluginApi& api) noexcept
{
    if (const auto error = detail::common_validation_error(api))
    {
        return error;
    }
    switch (api.kind)
    {
    case CALC_PLUGIN_FUNCTION:
        return detail::function_validation_error(api);
    case CALC_PLUGIN_INFIX:
        return detail::infix_validation_error(api);
    default:
        return "Unknown operation kind";
    }
}

} // namespace calc::plugin
