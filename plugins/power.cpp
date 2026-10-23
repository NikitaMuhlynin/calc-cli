#include <calc/plugin_sdk.h>

#include <cmath>
#include <stdexcept>

namespace
{

double power(const double* arguments, uint32_t)
{
    const double base     = arguments[0];
    const double exponent = arguments[1];
    if (base == 0.0 && exponent < 0.0)
    {
        throw std::domain_error("Zero cannot be raised to a negative power");
    }
    if (base < 0.0 && std::trunc(exponent) != exponent)
    {
        throw std::domain_error("Negative base requires an integer exponent");
    }
    return std::pow(base, exponent);
}

constexpr auto power_api = calc::plugin::infix<power>("^", 4, CALC_PLUGIN_ASSOC_RIGHT);

} // namespace

extern "C" CALC_PLUGIN_EXPORT CalcPluginStatus CALC_PLUGIN_CALL
calc_plugin_get_api(uint32_t version, uint32_t size, CalcPluginApi* output) noexcept
{
    return calc::plugin::publish(power_api, version, size, output);
}
