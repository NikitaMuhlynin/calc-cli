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
    char        symbol;
    std::size_t position;
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

int precedence(char symbol)
{
    switch (symbol)
    {
    case '~':
        return 3;
    case '*':
    case '/':
        return 2;
    case '+':
    case '-':
        return 1;
    default:
        return 0;
    }
}

Instruction instruction(PendingOperator pending)
{
    switch (pending.symbol)
    {
    case '+':
        return {Operation::add, 0.0, pending.position};
    case '-':
        return {Operation::subtract, 0.0, pending.position};
    case '*':
        return {Operation::multiply, 0.0, pending.position};
    case '/':
        return {Operation::divide, 0.0, pending.position};
    case '~':
        return {Operation::negate, 0.0, pending.position};
    default:
        syntax_error("Unclosed parenthesis", pending.position);
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

Expression parse(std::string_view input)
{
    Expression                   output;
    std::vector<PendingOperator> operators;
    bool                         expect_operand = true;
    std::size_t                  position       = 0;

    // Iterative shunting-yard parsing avoids recursion for nested parentheses
    // and long chains of unary operators.
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
        if (symbol == '(')
        {
            if (!expect_operand)
            {
                syntax_error("Expected an operator before parenthesis", position);
            }
            operators.push_back({symbol, position});
        }
        else if (symbol == ')')
        {
            if (expect_operand)
            {
                syntax_error("Expected an operand", position);
            }
            while (!operators.empty() && operators.back().symbol != '(')
            {
                output.push_back(instruction(operators.back()));
                operators.pop_back();
            }
            if (operators.empty())
            {
                syntax_error("Unmatched closing parenthesis", position);
            }
            operators.pop_back();
        }
        else if (symbol == '+' || symbol == '-' || symbol == '*' || symbol == '/')
        {
            if (expect_operand)
            {
                if (symbol == '-')
                {
                    operators.push_back({'~', position});
                }
                else if (symbol != '+')
                {
                    syntax_error("Expected an operand", position);
                }
            }
            else
            {
                while (!operators.empty() && operators.back().symbol != '(' &&
                       precedence(operators.back().symbol) >= precedence(symbol))
                {
                    output.push_back(instruction(operators.back()));
                    operators.pop_back();
                }
                operators.push_back({symbol, position});
                expect_operand = true;
            }
        }
        else
        {
            syntax_error("Unexpected character", position);
        }
        ++position;
    }

    if (expect_operand)
    {
        syntax_error("Expected an operand", position);
    }
    while (!operators.empty())
    {
        output.push_back(instruction(operators.back()));
        operators.pop_back();
    }
    return output;
}

} // namespace calc
