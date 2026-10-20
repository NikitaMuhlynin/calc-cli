#pragma once

#include "expression.h"

#include <string_view>

namespace calc {

Expression parse(std::string_view input);

} // namespace calc
