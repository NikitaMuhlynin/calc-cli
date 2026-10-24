#include <calc/plugin_sdk.h>

#include <cstring>
#include <limits>

namespace
{

// Deliberately violates parts of the callback contract to test host defenses.
CalcPluginStatus CALC_PLUGIN_CALL invoke(const double* arguments, uint32_t, double* result,
                                         char* error, uint32_t capacity) noexcept
{
    if (arguments[0] == 0)
    {
        return CALC_PLUGIN_OK; // A success code without a result must not become zero.
    }
    if (arguments[0] == 1)
    {
        *result = std::numeric_limits<double>::infinity();
        return CALC_PLUGIN_OK;
    }
    if (arguments[0] == 2)
    {
        if (error != nullptr)
        {
            std::memset(error, 'x', capacity); // Missing terminator, but within the buffer.
        }
        return CALC_PLUGIN_DOMAIN_ERROR;
    }
    if (arguments[0] == 3)
    {
        return CALC_PLUGIN_DOMAIN_ERROR; // Missing optional diagnostic text.
    }
    if (arguments[0] == 4)
    {
        return 999; // Unknown status.
    }
    *result = arguments[0];
    return CALC_PLUGIN_OK;
}

constexpr CalcPluginApi unreliable_api = {CALC_PLUGIN_ABI_VERSION,
                                          sizeof(CalcPluginApi),
                                          "unreliable",
                                          CALC_PLUGIN_FUNCTION,
                                          1,
                                          0,
                                          CALC_PLUGIN_ASSOC_NONE,
                                          &invoke};

} // namespace

extern "C" CALC_PLUGIN_EXPORT CalcPluginStatus CALC_PLUGIN_CALL
calc_plugin_get_api(uint32_t version, uint32_t size, CalcPluginApi* output) noexcept
{
    return calc::plugin::publish(unreliable_api, version, size, output);
}
