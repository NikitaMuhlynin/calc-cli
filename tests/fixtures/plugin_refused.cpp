#include <calc/plugin_api.h>

extern "C" CALC_PLUGIN_EXPORT CalcPluginStatus CALC_PLUGIN_CALL
calc_plugin_get_api(uint32_t, uint32_t, CalcPluginApi*) noexcept
{
    return CALC_PLUGIN_INCOMPATIBLE_ABI;
}
