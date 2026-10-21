#include <calc/plugin_api.h>

_Static_assert(sizeof(CalcPluginStatus) == 4, "Status must use 32 bits");
_Static_assert(sizeof(((CalcPluginApi*)0)->name) == 64, "Name capacity is part of ABI v1");
