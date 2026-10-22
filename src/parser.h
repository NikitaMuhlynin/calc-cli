#pragma once

#include "expression.h"
#include "plugin_registry.h"

#include <string_view>

namespace calc
{

Expression parse(std::string_view input, const PluginRegistry& plugins = {});

} // namespace calc
