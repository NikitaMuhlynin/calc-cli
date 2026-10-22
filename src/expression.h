#pragma once

#include <cstddef>
#include <vector>

namespace calc
{

enum class Operation
{
    number,
    add,
    subtract,
    multiply,
    divide,
    negate
};

struct Instruction
{
    Operation   operation;
    double      value;
    std::size_t position;
};

// Reverse Polish notation: each operator follows its operands.
using Expression = std::vector<Instruction>;

} // namespace calc
