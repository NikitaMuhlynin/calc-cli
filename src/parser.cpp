#include "parser.h"

#include <cctype>
#include <charconv>
#include <stdexcept>
#include <string>
#include <system_error>
#include <utility>

namespace calc
{
namespace
{

struct PendingOperator
{
    Instruction item{};
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

bool is_identifier_start(char character)
{
    return (character >= 'a' && character <= 'z') || (character >= 'A' && character <= 'Z') ||
           character == '_';
}

bool is_identifier_character(char character)
{
    return is_identifier_start(character) || is_digit(character);
}

bool matches(std::string_view input, std::size_t position, char character)
{
    return position < input.size() && input[position] == character;
}

void skip_whitespace(std::string_view input, std::size_t& position)
{
    while (position < input.size() && std::isspace(static_cast<unsigned char>(input[position])))
    {
        ++position;
    }
}

std::size_t read_digits(std::string_view input, std::size_t& position)
{
    const auto start = position;
    while (position < input.size() && is_digit(input[position]))
    {
        ++position;
    }
    return position - start;
}

void read_exponent(std::string_view input, std::size_t& position)
{
    if (!matches(input, position, 'e') && !matches(input, position, 'E'))
    {
        return;
    }
    ++position;
    if (matches(input, position, '+') || matches(input, position, '-'))
    {
        ++position;
    }
    if (read_digits(input, position) == 0)
    {
        syntax_error("Expected exponent digits", position);
    }
}

Instruction read_number(std::string_view input, std::size_t& position)
{
    const auto start  = position;
    auto       digits = read_digits(input, position);
    if (matches(input, position, '.'))
    {
        ++position;
        digits += read_digits(input, position);
    }
    if (digits == 0)
    {
        syntax_error("Expected a number", start);
    }
    read_exponent(input, position);

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

bool should_emit(const PendingOperator& pending, const PendingOperator& incoming)
{
    return pending.precedence != 0 &&
           (pending.precedence > incoming.precedence ||
            (pending.precedence == incoming.precedence && !incoming.right_associative));
}

// Each handler consumes one syntactic construct. The operator stack keeps
// parentheses and function calls iterative, including deeply nested input.
class Parser
{
  public:
    Parser(std::string_view input, const PluginRegistry& plugins) : input_(input), plugins_(plugins)
    {
    }

    Expression run()
    {
        while (position_ < input_.size())
        {
            skip_whitespace(input_, position_);
            if (position_ == input_.size())
            {
                break;
            }
            read_next();
        }
        return finish();
    }

  private:
    void require_operand_position(const char* message) const
    {
        if (!expect_operand_)
        {
            syntax_error(message, position_);
        }
    }

    void require_completed_operand(const char* message) const
    {
        if (expect_operand_)
        {
            syntax_error(message, position_);
        }
    }

    void emit_operator()
    {
        output_.push_back(operators_.back().item);
        operators_.pop_back();
    }

    void emit_until_parenthesis()
    {
        while (!operators_.empty() && operators_.back().precedence != 0)
        {
            emit_operator();
        }
    }

    void read_value()
    {
        require_operand_position("Expected an operator");
        output_.push_back(read_number(input_, position_));
        expect_operand_ = false;
    }

    void read_function()
    {
        require_operand_position("Expected an operator");
        const auto start = position_++;
        while (position_ < input_.size() && is_identifier_character(input_[position_]))
        {
            ++position_;
        }
        const auto function = plugins_.find(input_.substr(start, position_ - start));
        if (!function || function->api.kind != CALC_PLUGIN_FUNCTION)
        {
            syntax_error("Unknown function", start);
        }
        skip_whitespace(input_, position_);
        if (!matches(input_, position_, '('))
        {
            syntax_error("Expected function parentheses", position_);
        }
        operators_.push_back({{Operation::plugin_call, 0.0, start, function}});
        ++position_;
    }

    void open_parenthesis()
    {
        require_operand_position("Expected an operator before parenthesis");
        operators_.push_back({{Operation::number, 0.0, position_}});
    }

    void close_parenthesis()
    {
        require_completed_operand("Expected an operand");
        emit_until_parenthesis();
        if (operators_.empty())
        {
            syntax_error("Unmatched closing parenthesis", position_);
        }
        const auto frame = operators_.back();
        operators_.pop_back();
        if (frame.item.plugin)
        {
            if (frame.separators + 1 != frame.item.plugin->api.argument_count)
            {
                syntax_error("Incorrect function argument count", frame.item.position);
            }
            output_.push_back(frame.item);
        }
    }

    void separate_argument()
    {
        require_completed_operand("Expected an argument");
        emit_until_parenthesis();
        if (operators_.empty() || !operators_.back().item.plugin)
        {
            syntax_error("Unexpected argument separator", position_);
        }
        auto& frame = operators_.back();
        if (++frame.separators >= frame.item.plugin->api.argument_count)
        {
            syntax_error("Too many function arguments", position_);
        }
        expect_operand_ = true;
    }

    PendingOperator binary_operator(char symbol) const
    {
        if (symbol == '+' || symbol == '-' || symbol == '*' || symbol == '/')
        {
            return builtin(symbol, position_);
        }
        const auto operation = plugins_.find(input_.substr(position_, 1));
        if (!operation || operation->api.kind != CALC_PLUGIN_INFIX)
        {
            syntax_error("Unexpected character", position_);
        }
        return {{Operation::plugin_call, 0.0, position_, operation},
                operation->api.precedence,
                operation->api.associativity == CALC_PLUGIN_ASSOC_RIGHT};
    }

    void read_operator(char symbol)
    {
        if (expect_operand_)
        {
            if (symbol == '-')
            {
                operators_.push_back({{Operation::negate, 0.0, position_}, 3, true});
            }
            else if (symbol != '+')
            {
                syntax_error("Expected an operand", position_);
            }
            return;
        }
        const auto current = binary_operator(symbol);
        while (!operators_.empty() && should_emit(operators_.back(), current))
        {
            emit_operator();
        }
        operators_.push_back(current);
        expect_operand_ = true;
    }

    void read_next()
    {
        const char symbol = input_[position_];
        if (is_digit(symbol) || symbol == '.')
        {
            read_value();
            return;
        }
        if (is_identifier_start(symbol))
        {
            read_function();
            return;
        }
        switch (symbol)
        {
        case '(':
            open_parenthesis();
            break;
        case ')':
            close_parenthesis();
            break;
        case ',':
            separate_argument();
            break;
        default:
            read_operator(symbol);
            break;
        }
        ++position_;
    }

    Expression finish()
    {
        require_completed_operand("Expected an operand");
        while (!operators_.empty())
        {
            if (operators_.back().precedence == 0)
            {
                syntax_error("Unclosed parenthesis", operators_.back().item.position);
            }
            emit_operator();
        }
        return std::move(output_);
    }

    std::string_view             input_;
    const PluginRegistry&        plugins_;
    Expression                   output_{};
    std::vector<PendingOperator> operators_{};
    std::size_t                  position_       = 0;
    bool                         expect_operand_ = true;
};

} // namespace

Expression parse(std::string_view input, const PluginRegistry& plugins)
{
    return Parser(input, plugins).run();
}

} // namespace calc
