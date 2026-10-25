#include "evaluator.h"
#include "parser.h"
#include "plugin_registry.h"

#include <gtest/gtest.h>

#include <sstream>
#include <stdexcept>

namespace
{

class Plugins : public testing::Test
{
  protected:
    calc::PluginRegistry registry;

    void SetUp() override
    {
        std::ostringstream diagnostics;
        registry = calc::load_plugins(calc::executable_directory() / "plugins", diagnostics);
        ASSERT_TRUE(diagnostics.str().empty()) << diagnostics.str();
        ASSERT_NE(registry.find("sin"), nullptr);
        ASSERT_NE(registry.find("^"), nullptr);
    }
};

TEST_F(Plugins, EvaluatesDegreeSineAndRightAssociativePower)
{
    struct Case
    {
        const char* expression;
        double      expected;
    };
    const Case cases[] = {{"2^4 + sin(90)", 17},
                          {"2^3^2", 512},
                          {"-2^2", -4},
                          {"(-2)^2", 4},
                          {"2^-2", 0.25},
                          {"2^-2^2", 0.0625},
                          {"2^3*4+1", 33},
                          {"2*3^2", 18},
                          {"(-2)^3", -8},
                          {"0^0", 1},
                          {"sin(-90)", -1},
                          {"sin(180)", 0},
                          {"sin(450)", 1},
                          {"sin(30)", 0.5},
                          {"sin(45)", 0.7071067811865476},
                          {"sin(360000030)", 0.5},
                          {"2^0.5", 1.4142135623730951},
                          {"4^-0.5", 0.5},
                          {"2^sin(90)+sin(30)", 2.5}};
    for (const auto& item : cases)
    {
        SCOPED_TRACE(item.expression);
        EXPECT_NEAR(calc::evaluate(calc::parse(item.expression, registry)), item.expected, 1e-12);
    }
}

TEST_F(Plugins, RejectsInvalidCallsAndNumericErrors)
{
    for (const char* expression : {"sin()", "sin(1,2)", "sin(1,)", "sin 2", "2^"})
    {
        SCOPED_TRACE(expression);
        EXPECT_THROW(calc::parse(expression, registry), std::invalid_argument);
    }
    for (const char* expression : {"(-2)^0.5", "0^-1", "1e308^2"})
    {
        SCOPED_TRACE(expression);
        EXPECT_THROW(calc::evaluate(calc::parse(expression, registry)), std::runtime_error);
    }
    EXPECT_DOUBLE_EQ(calc::evaluate(calc::parse("sin(90)", registry)), 1.0);
}

} // namespace
