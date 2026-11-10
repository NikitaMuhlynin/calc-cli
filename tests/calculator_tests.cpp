#include "console.h"
#include "evaluator.h"
#include "parser.h"

#include <gtest/gtest.h>

#include <sstream>
#include <stdexcept>
#include <string>

namespace
{

TEST(Arithmetic, EvaluatesNumbersAndWhitespace)
{
    EXPECT_DOUBLE_EQ(calc::evaluate(calc::parse(" \t1.5 + .5\t")), 2.0);
    EXPECT_DOUBLE_EQ(calc::evaluate(calc::parse("5./2")), 2.5);
    EXPECT_DOUBLE_EQ(calc::evaluate(calc::parse("1e2+2.5E-1*4")), 101.0);
    EXPECT_DOUBLE_EQ(calc::evaluate(calc::parse("-.5e+2")), -50.0);
    EXPECT_DOUBLE_EQ(calc::evaluate(calc::parse("5.e-1")), 0.5);
}

TEST(Arithmetic, RespectsPrecedenceParenthesesAndUnarySigns)
{
    struct Case
    {
        const char* expression;
        double      expected;
    };
    const Case cases[] = {{"16 + 4 * (3 - 1)", 24},
                          {"(2+3)*4", 20},
                          {"8/2*3", 12},
                          {"10-3-2", 5},
                          {"-2*3", -6},
                          {"-(2+3)", -5},
                          {"2*-3", -6},
                          {"--2", 2},
                          {"2--3", 5},
                          {"2/-2", -1},
                          {"+2", 2}};
    for (const auto& item : cases)
    {
        SCOPED_TRACE(item.expression);
        EXPECT_DOUBLE_EQ(calc::evaluate(calc::parse(item.expression)), item.expected);
    }
}

TEST(Parser, RejectsMalformedExpressions)
{
    for (const char* expression :
         {"", "1+", "*2", "()", "(1+2", "1+2)", "2(3)", "1 2", "1..2", ".", "1e+", "1e", "1e-",
          ".e2", "1e2.3", "unknown(1)", "1,5", "1e309"})
    {
        SCOPED_TRACE(expression);
        EXPECT_THROW(calc::parse(expression), std::invalid_argument);
    }
}

TEST(Parser, HandlesDeepNestingAndUnaryChains)
{
    const std::string nested = std::string(2000, '(') + "2" + std::string(2000, ')');
    EXPECT_DOUBLE_EQ(calc::evaluate(calc::parse(nested)), 2.0);
    EXPECT_DOUBLE_EQ(calc::evaluate(calc::parse(std::string(2000, '-') + "2")), 2.0);
}

TEST(Evaluator, RejectsDivisionByZeroAndOverflow)
{
    EXPECT_THROW(calc::evaluate(calc::parse("2/(1-1)")), std::domain_error);
    EXPECT_THROW(calc::evaluate(calc::parse("1/(-0)")), std::domain_error);
    EXPECT_THROW(calc::evaluate(calc::parse("1e308*10")), std::overflow_error);
}

TEST(Console, HandlesEmptyInput)
{
    std::istringstream input;
    std::ostringstream output;
    std::ostringstream errors;
    EXPECT_EQ(calc::run_console(input, output, errors), 0);
    EXPECT_TRUE(output.str().empty());
    EXPECT_TRUE(errors.str().empty());
}

TEST(Console, ContinuesAfterErrorsAndAcceptsFinalLineWithoutNewline)
{
    std::istringstream input(" \n1+\n2+2\n1/0\n3*3");
    std::ostringstream output;
    std::ostringstream errors;
    EXPECT_EQ(calc::run_console(input, output, errors), 1);
    EXPECT_EQ(output.str(), "4\n9\n");
    EXPECT_NE(errors.str().find("Division by zero"), std::string::npos);
    EXPECT_NE(errors.str().find("Expected an operand"), std::string::npos);
}

} // namespace
