#pragma once

#include "plugin_api.h"

#include <cstring>

namespace calc::plugin {

inline const char* validation_error(const CalcPluginApi& api) noexcept
{
    if (api.abi_version != CALC_PLUGIN_ABI_VERSION || api.struct_size != sizeof(CalcPluginApi)) {
        return "Incompatible plugin ABI";
    }
    if (api.invoke == nullptr || api.argument_count == 0 ||
        api.argument_count > CALC_PLUGIN_MAX_ARGUMENTS) {
        return "Invalid callback or argument count";
    }
    if (api.name[0] == '\0' || std::memchr(api.name, '\0', sizeof(api.name)) == nullptr) {
        return "Missing or unterminated operation name";
    }
    if (api.kind == CALC_PLUGIN_FUNCTION) {
        if (api.precedence != 0 || api.associativity != CALC_PLUGIN_ASSOC_NONE) {
            return "Named functions have no precedence or associativity";
        }
        for (uint32_t index = 0; api.name[index] != '\0'; ++index) {
            const char character = api.name[index];
            const bool letter = (character >= 'a' && character <= 'z') ||
                                (character >= 'A' && character <= 'Z') || character == '_';
            if (!letter && !(index != 0 && character >= '0' && character <= '9')) {
                return "Invalid function identifier";
            }
        }
    } else if (api.kind == CALC_PLUGIN_INFIX) {
        if (api.name[1] != '\0' || std::strchr("^%&|~!@?:", api.name[0]) == nullptr) {
            return "Invalid or reserved operator symbol";
        }
        if (api.argument_count != 2 || api.precedence == 0 || api.precedence > 100 ||
            (api.associativity != CALC_PLUGIN_ASSOC_LEFT &&
             api.associativity != CALC_PLUGIN_ASSOC_RIGHT)) {
            return "Invalid binary operator parsing rules";
        }
    } else {
        return "Unknown operation kind";
    }
    return nullptr;
}

} // namespace calc::plugin
