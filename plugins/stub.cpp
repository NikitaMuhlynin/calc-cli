#include <calc/plugin_sdk.h>

namespace
{

double identity(const double* arguments, uint32_t)
{
    return arguments[0];
}

constexpr auto identity_api = calc::plugin::function<1, identity>("identity");

} // namespace

extern "C" CALC_PLUGIN_EXPORT CalcPluginStatus CALC_PLUGIN_CALL calc_plugin_get_api(
    uint32_t requested_version, uint32_t output_size, CalcPluginApi* output) noexcept
{
    return calc::plugin::publish(identity_api, requested_version, output_size, output);
}
