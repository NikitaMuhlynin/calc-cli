#include <calc/plugin_sdk.h>

namespace
{
double difference(const double* arguments, uint32_t)
{
    return arguments[0] - arguments[1];
}
constexpr auto difference_api = calc::plugin::infix<difference>("?", 2, CALC_PLUGIN_ASSOC_LEFT);
} // namespace

extern "C" CALC_PLUGIN_EXPORT CalcPluginStatus CALC_PLUGIN_CALL
calc_plugin_get_api(uint32_t version, uint32_t size, CalcPluginApi* output) noexcept
{
    return calc::plugin::publish(difference_api, version, size, output);
}
