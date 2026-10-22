#include "parser.h"

#include <cctype>
#include <charconv>
#include <stdexcept>
#include <string>
#include <system_error>

namespace calc
{
namespace
{

struct PendingOperator
{
    Instruction item;
    uint32_t    precedence        = 0; // Zero marks a grouping or function parenthesis.
    bool        right_associative = false;
    uint32_t    separators        = 0;
};

[[noreturn]] void syntax_error(const char* message, std::size_t position)
{
    throw std::invalid_argument(std::string(message) + " at position " +
                                std::to_string(position + 1));
}

bool is_digit(char character)
{
    return character >= '0' && character <= '9';
}

PendingOperator builtin(char symbol, std::size_t position)
{
    switch (symbol)
    {
    case '+':
        return {{Operation::add, 0.0, position}, 1};
    case '-':
        return {{Operation::subtract, 0.0, position}, 1};
    case '*':
        return {{Operation::multiply, 0.0, position}, 2};
    case '/':
        return {{Operation::divide, 0.0, position}, 2};
    default:
        syntax_error("Unknown operator", position);
    }
}

Instruction read_number(std::string_view input, std::size_t& position)
{
    const auto start      = position;
    bool       has_digits = false;
    while (position < input.size() && is_digit(input[position]))
    {
        has_digits = true;
        ++position;
    }
    if (position < input.size() && input[position] == '.')
    {
        ++position;
        while (position < input.size() && is_digit(input[position]))
        {
            has_digits = true;
            ++position;
        }
    }
    if (!has_digits)
    {
        syntax_error("Expected a number", start);
    }
    if (position < input.size() && (input[position] == 'e' || input[position] == 'E'))
    {
        ++position;
        if (position < input.size() && (input[position] == '+' || input[position] == '-'))
        {
            ++position;
        }
        const auto exponent_start = position;
        while (position < input.size() && is_digit(input[position]))
        {
            ++position;
        }
        if (position == exponent_start)
        {
            syntax_error("Expected exponent digits", position);
        }
    }

    double      value  = 0.0;
    const char* begin  = input.data() + start;
    const char* end    = input.data() + position;
    const auto  result = std::from_chars(begin, end, value, std::chars_format::general);
    if (result.ec == std::errc::result_out_of_range)
    {
        syntax_error("Number is out of range", start);
    }
    if (result.ec != std::errc{} || result.ptr != end)
    {
        syntax_error("Invalid number", start);
    }
    return {Operation::number, value, start};
}

} // namespace

Expression parse(std::string_view input, const PluginRegistry& plugins)
{
    Expression                   output;
    std::vector<PendingOperator> operators;
    bool                         expect_operand = true;
    std::size_t                  position       = 0;

    const auto emit_operator = [&]()
    {
        output.push_back(operators.back().item);
        operators.pop_back();
    };

    // Iterative shunting-yard parsing includes function frames and dynamic operators.
    while (position < input.size())
    {
        const char symbol = input[position];
        if (std::isspace(static_cast<unsigned char>(symbol)))
        {
            ++position;
            continue;
        }
        if (is_digit(symbol) || symbol == '.')
        {
            if (!expect_operand)
            {
                syntax_error("Expected an operator", position);
            }
            output.push_back(read_number(input, position));
            expect_operand = false;
            continue;
        }
        if ((symbol >= 'a' && symbol <= 'z') || (symbol >= 'A' && symbol <= 'Z') || symbol == '_')
        {
            if (!expect_operand)
            {
                syntax_error("Expected an operator", position);
            }
            const auto start = position++;
            while (position < input.size())
            {
                const char character = input[position];
                if (!is_digit(character) && !(character >= 'a' && character <= 'z') &&
                    !(character >= 'A' && character <= 'Z') && character != '_')
                {
                    break;
                }
                ++position;
            }
            const auto function = plugins.find(input.substr(start, position - start));
            if (!function || function->api.kind != CALC_PLUGIN_FUNCTION)
            {
                syntax_error("Unknown function", start);
            }
            while (position < input.size() &&
                   std::isspace(static_cast<unsigned char>(input[position])))
            {
                ++position;
            }
            if (position == input.size() || input[position] != '(')
            {
                syntax_error("Expected function parentheses", position);
            }
            operators.push_back({{Operation::plugin_call, 0.0, start, function}});
        }
        else if (symbol == '(')
        {
            if (!expect_operand)
            {
                syntax_error("Expected an operator before parenthesis", position);
            }
            operators.push_back({{Operation::number, 0.0, position}});
        }
        else if (symbol == ')')
        {
            if (expect_operand)
            {
                syntax_error("Expected an operand", position);
            }
            while (!operators.empty() && operators.back().precedence != 0)
            {
                emit_operator();
            }
            if (operators.empty())
            {
                syntax_error("Unmatched closing parenthesis", position);
            }
            const auto frame = operators.back();
            operators.pop_back();
            if (frame.item.plugin)
            {
                if (frame.separators + 1 != frame.item.plugin->api.argument_count)
                {
                    syntax_error("Incorrect function argument count", frame.item.position);
                }
                output.push_back(frame.item);
            }
        }
        else if (symbol == ',')
        {
            if (expect_operand)
            {
                syntax_error("Expected an argument", position);
            }
            while (!operators.empty() && operators.back().precedence != 0)
            {
                emit_operator();
            }
            if (operators.empty() || !operators.back().item.plugin)
            {
                syntax_error("Unexpected argument separator", position);
            }
            if (++operators.back().separators >= operators.back().item.plugin->api.argument_count)
            {
                syntax_error("Too many function arguments", position);
            }
            expect_operand = true;
        }
        else
        {
            if (expect_operand)
            {
                if (symbol == '-')
                {
                    operators.push_back({{Operation::negate, 0.0, position}, 3, true});
                }
                else if (symbol != '+')
                {
                    syntax_error("Expected an operand", position);
                }
            }
            else
            {
                PendingOperator current{};
                if (symbol == '+' || symbol == '-' || symbol == '*' || symbol == '/')
                {
                    current = builtin(symbol, position);
                }
                else
                {
                    const auto operation = plugins.find(input.substr(position, 1));
                    if (!operation || operation->api.kind != CALC_PLUGIN_INFIX)
                    {
                        syntax_error("Unexpected character", position);
                    }
                    current = {{Operation::plugin_call, 0.0, position, operation},
                               operation->api.precedence,
                               operation->api.associativity == CALC_PLUGIN_ASSOC_RIGHT};
                }
                while (!operators.empty() && operators.back().precedence != 0 &&
                       (operators.back().precedence > current.precedence ||
                        (operators.back().precedence == current.precedence &&
                         !current.right_associative)))
                {
                    emit_operator();
                }
                operators.push_back(current);
                expect_operand = true;
            }
        }
        ++position;
    }

    if (expect_operand)
    {
        syntax_error("Expected an operand", position);
    }
    while (!operators.empty())
    {
        if (operators.back().precedence == 0)
        {
            syntax_error("Unclosed parenthesis", operators.back().item.position);
        }
        emit_operator();
    }
    return output;
}

} // namespace calc
