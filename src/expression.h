#pragma once

#include <cstddef>
#include <memory>
#include <vector>

namespace calc
{

struct PluginOperation;

enum class Operation
{
    number,
    add,
    subtract,
    multiply,
    divide,
    negate,
    plugin_call
};

struct Instruction
{
    Operation                              operation = Operation::number;
    double                                 value     = 0.0;
    std::size_t                            position  = 0;
    std::shared_ptr<const PluginOperation> plugin    = {};
};

// Reverse Polish notation: each operator follows its operands.
using Expression = std::vector<Instruction>;

} // namespace calc
