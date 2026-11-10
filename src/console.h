#pragma once

#include "plugin_registry.h"

#include <iosfwd>

namespace calc
{

int run_console(std::istream& input, std::ostream& output, std::ostream& errors,
                const PluginRegistry& plugins = {});

} // namespace calc
