#include <calc/plugin_api.h>

extern "C" CALC_PLUGIN_EXPORT CalcPluginStatus CALC_PLUGIN_CALL
calc_plugin_get_api(uint32_t, uint32_t, CalcPluginApi* output) noexcept
{
    *output = {};
    return CALC_PLUGIN_OK;
}
