#include <calc/plugin_sdk.h>

#include <cmath>

namespace
{

double sine_degrees(const double* arguments, uint32_t)
{
    const double degrees = std::remainder(arguments[0], 360.0);
    // Exact cardinal angles avoid rounding residues such as sin(180) != 0.
    if (degrees == 0.0 || std::abs(degrees) == 180.0)
    {
        return 0.0;
    }
    if (degrees == 90.0)
    {
        return 1.0;
    }
    if (degrees == -90.0)
    {
        return -1.0;
    }
    const double radians_per_degree = std::acos(-1.0) / 180.0;
    return std::sin(degrees * radians_per_degree);
}

constexpr auto sine_api = calc::plugin::function<1, sine_degrees>("sin");

} // namespace

extern "C" CALC_PLUGIN_EXPORT CalcPluginStatus CALC_PLUGIN_CALL
calc_plugin_get_api(uint32_t version, uint32_t size, CalcPluginApi* output) noexcept
{
    return calc::plugin::publish(sine_api, version, size, output);
}
