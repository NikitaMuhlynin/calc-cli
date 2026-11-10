#pragma once

#include "plugin_api.h"
#include "plugin_validation.h"

#include <cmath>
#include <cstddef>
#include <exception>
#include <stdexcept>

namespace calc::plugin
{
namespace detail
{

inline CalcPluginStatus fail(CalcPluginStatus status, const char* message, char* buffer,
                             uint32_t capacity) noexcept
{
    if (buffer != nullptr && capacity != 0)
    {
        uint32_t index = 0;
        while (index < capacity - 1 && message[index] != '\0')
        {
            buffer[index] = message[index];
            ++index;
        }
        buffer[index] = '\0';
    }
    return status;
}

template <uint32_t Arity, auto Function>
CalcPluginStatus CALC_PLUGIN_CALL invoke(const double* arguments, uint32_t count, double* result,
                                         char* error, uint32_t capacity) noexcept
{
    if (error != nullptr && capacity != 0)
    {
        error[0] = '\0';
    }
    if (arguments == nullptr || result == nullptr || count != Arity ||
        (error == nullptr && capacity != 0))
    {
        return fail(CALC_PLUGIN_INVALID_ARGUMENT, "Invalid invocation arguments", error, capacity);
    }
    for (uint32_t index = 0; index < count; ++index)
    {
        if (!std::isfinite(arguments[index]))
        {
            return fail(CALC_PLUGIN_NUMERIC_ERROR, "Non-finite argument", error, capacity);
        }
    }
    try
    {
        const double value = Function(arguments, count);
        if (!std::isfinite(value))
        {
            return fail(CALC_PLUGIN_NUMERIC_ERROR, "Non-finite result", error, capacity);
        }
        *result = value;
        return CALC_PLUGIN_OK;
    }
    catch (const std::domain_error& exception)
    {
        return fail(CALC_PLUGIN_DOMAIN_ERROR, exception.what(), error, capacity);
    }
    catch (const std::invalid_argument& exception)
    {
        return fail(CALC_PLUGIN_INVALID_ARGUMENT, exception.what(), error, capacity);
    }
    catch (const std::overflow_error& exception)
    {
        return fail(CALC_PLUGIN_NUMERIC_ERROR, exception.what(), error, capacity);
    }
    catch (const std::underflow_error& exception)
    {
        return fail(CALC_PLUGIN_NUMERIC_ERROR, exception.what(), error, capacity);
    }
    catch (const std::range_error& exception)
    {
        return fail(CALC_PLUGIN_NUMERIC_ERROR, exception.what(), error, capacity);
    }
    catch (const std::exception& exception)
    {
        return fail(CALC_PLUGIN_INTERNAL_ERROR, exception.what(), error, capacity);
    }
    catch (...)
    {
        return fail(CALC_PLUGIN_INTERNAL_ERROR, "Unknown plugin exception", error, capacity);
    }
}

} // namespace detail

template <uint32_t Arity, auto Function, std::size_t NameSize>
constexpr CalcPluginApi function(const char (&name)[NameSize]) noexcept
{
    static_assert(Arity >= 1 && Arity <= CALC_PLUGIN_MAX_ARGUMENTS, "Unsupported arity");
    static_assert(NameSize > 1 && NameSize <= CALC_PLUGIN_NAME_CAPACITY, "Invalid name length");
    CalcPluginApi api{};
    api.abi_version = CALC_PLUGIN_ABI_VERSION;
    api.struct_size = sizeof(CalcPluginApi);
    for (std::size_t index = 0; index < NameSize; ++index)
    {
        api.name[index] = name[index];
    }
    api.kind           = CALC_PLUGIN_FUNCTION;
    api.argument_count = Arity;
    api.associativity  = CALC_PLUGIN_ASSOC_NONE;
    api.invoke         = &detail::invoke<Arity, Function>;
    return api;
}

template <auto Function, std::size_t NameSize>
constexpr CalcPluginApi infix(const char (&symbol)[NameSize], uint32_t precedence,
                              uint32_t associativity) noexcept
{
    static_assert(NameSize == 2, "An infix operator must have a single ASCII symbol");
    auto api          = function<2, Function>(symbol);
    api.kind          = CALC_PLUGIN_INFIX;
    api.precedence    = precedence;
    api.associativity = associativity;
    return api;
}

inline CalcPluginStatus publish(const CalcPluginApi& api, uint32_t requested_version,
                                uint32_t output_size, CalcPluginApi* output) noexcept
{
    if (output == nullptr)
    {
        return CALC_PLUGIN_INVALID_ARGUMENT;
    }
    if (requested_version != CALC_PLUGIN_ABI_VERSION || output_size != sizeof(CalcPluginApi))
    {
        return CALC_PLUGIN_INCOMPATIBLE_ABI;
    }
    if (validation_error(api) != nullptr)
    {
        return CALC_PLUGIN_INVALID_ARGUMENT;
    }
    *output = api;
    return CALC_PLUGIN_OK;
}

} // namespace calc::plugin
