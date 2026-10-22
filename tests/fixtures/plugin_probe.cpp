#include <calc/plugin_sdk.h>

#include <limits>
#include <stdexcept>

namespace
{

// Negative sentinel inputs exercise error paths inside an actual DLL.
double probe(const double* arguments, uint32_t)
{
    const double value = arguments[0];
    if (value == -1)
    {
        throw std::domain_error("Probe domain error");
    }
    if (value == -2)
    {
        throw std::overflow_error("Probe numeric error");
    }
    if (value == -3)
    {
        throw std::runtime_error("Probe runtime error");
    }
    if (value == -4)
    {
        throw 42;
    }
    if (value == -5)
    {
        return std::numeric_limits<double>::infinity();
    }
    if (value == -6)
    {
        throw std::invalid_argument("Probe argument error");
    }
    if (value == -7)
    {
        throw std::underflow_error("Probe underflow error");
    }
    if (value == -8)
    {
        throw std::range_error("Probe range error");
    }
    return value * value;
}

constexpr auto probe_api = calc::plugin::function<1, probe>("probe");

} // namespace

extern "C" CALC_PLUGIN_EXPORT CalcPluginStatus CALC_PLUGIN_CALL calc_plugin_get_api(
    uint32_t requested_version, uint32_t output_size, CalcPluginApi* output) noexcept
{
    return calc::plugin::publish(probe_api, requested_version, output_size, output);
}
