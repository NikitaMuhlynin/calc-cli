#include <calc/plugin_sdk.h>

namespace
{
double difference(const double* arguments, uint32_t)
{
    return arguments[0] - arguments[1];
}
constexpr auto pair_api = calc::plugin::function<2, difference>("difference");
} // namespace

extern "C" CALC_PLUGIN_EXPORT CalcPluginStatus CALC_PLUGIN_CALL
calc_plugin_get_api(uint32_t version, uint32_t size, CalcPluginApi* output) noexcept
{
    return calc::plugin::publish(pair_api, version, size, output);
}
