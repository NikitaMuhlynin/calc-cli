#include "evaluator.h"
#include "plugin_registry.h"

#include <cmath>
#include <stdexcept>
#include <string>

namespace calc
{

double evaluate(const Expression& expression)
{
    std::vector<double> values;
    values.reserve(expression.size());

    for (const auto& item : expression)
    {
        if (item.operation == Operation::number)
        {
            if (!std::isfinite(item.value))
            {
                throw std::invalid_argument("Non-finite operand");
            }
            values.push_back(item.value);
            continue;
        }

        if (item.operation == Operation::plugin_call)
        {
            if (!item.plugin || values.size() < item.plugin->api.argument_count)
            {
                throw std::invalid_argument("Invalid plugin instruction");
            }
            const auto offset = values.size() - item.plugin->api.argument_count;
            double     result = 0.0;
            try
            {
                result =
                    item.plugin->evaluate(values.data() + offset, item.plugin->api.argument_count);
            }
            catch (const std::exception& error)
            {
                throw std::runtime_error(std::string(error.what()) + " at position " +
                                         std::to_string(item.position + 1));
            }
            values.resize(offset);
            values.push_back(result);
            continue;
        }
        const std::size_t operand_count = item.operation == Operation::negate ? 1 : 2;
        if (values.size() < operand_count)
        {
            throw std::invalid_argument("Not enough operands");
        }
        if (item.operation == Operation::negate)
        {
            values.back() = -values.back();
            continue;
        }

        const double right = values.back();
        values.pop_back();
        const double left   = values.back();
        double       result = 0.0;
        switch (item.operation)
        {
        case Operation::add:
            result = left + right;
            break;
        case Operation::subtract:
            result = left - right;
            break;
        case Operation::multiply:
            result = left * right;
            break;
        case Operation::divide:
            if (right == 0.0)
            {
                throw std::domain_error("Division by zero at position " +
                                        std::to_string(item.position + 1));
            }
            result = left / right;
            break;
        default:
            throw std::invalid_argument("Unknown operation");
        }
        if (!std::isfinite(result))
        {
            throw std::overflow_error("Result is out of range at position " +
                                      std::to_string(item.position + 1));
        }
        values.back() = result;
    }

    if (values.size() != 1)
    {
        throw std::invalid_argument("Expected a single result");
    }
    return values.back();
}

} // namespace calc
