#include <calc/plugin_sdk.h>

#include <gtest/gtest.h>

#include <limits>
#include <stdexcept>

namespace
{

double identity(const double* arguments, uint32_t)
{
    return arguments[0];
}

double failing(const double* arguments, uint32_t)
{
    if (arguments[0] == -1)
    {
        throw std::domain_error("Outside domain");
    }
    if (arguments[0] == -2)
    {
        // Exercise the SDK catch-all handler for a non-standard exception.
        throw 42; // NOLINT(bugprone-std-exception-baseclass)
    }
    return std::numeric_limits<double>::infinity();
}

TEST(PluginSdk, ValidatesMetadataAndAbiBeforePublishing)
{
    constexpr auto descriptor = calc::plugin::function<1, identity>("identity");
    CalcPluginApi  output{};
    ASSERT_EQ(calc::plugin::publish(descriptor, 1, sizeof(output), &output), CALC_PLUGIN_OK);
    EXPECT_STREQ(output.name, "identity");
    EXPECT_EQ(calc::plugin::publish(descriptor, 999, sizeof(output), &output),
              CALC_PLUGIN_INCOMPATIBLE_ABI);
    EXPECT_EQ(calc::plugin::publish(descriptor, 1, sizeof(output) - 1, &output),
              CALC_PLUGIN_INCOMPATIBLE_ABI);
    EXPECT_EQ(calc::plugin::publish(descriptor, 1, sizeof(output), nullptr),
              CALC_PLUGIN_INVALID_ARGUMENT);

    auto invalid          = descriptor;
    invalid.invoke        = nullptr;
    output.argument_count = 99;
    EXPECT_EQ(calc::plugin::publish(invalid, 1, sizeof(output), &output),
              CALC_PLUGIN_INVALID_ARGUMENT);
    EXPECT_EQ(output.argument_count, 99u);
    invalid         = descriptor;
    invalid.name[0] = '2';
    EXPECT_NE(calc::plugin::validation_error(invalid), nullptr);
    invalid            = descriptor;
    invalid.precedence = 1;
    EXPECT_NE(calc::plugin::validation_error(invalid), nullptr);
}

TEST(PluginSdk, RejectsInvalidArgumentsWithoutChangingTheResult)
{
    constexpr auto descriptor = calc::plugin::function<1, identity>("identity");
    double         argument   = 3.5;
    double         result     = 123;
    char           error[64]  = "stale error";
    EXPECT_EQ(descriptor.invoke(&argument, 0, &result, error, sizeof(error)),
              CALC_PLUGIN_INVALID_ARGUMENT);
    EXPECT_DOUBLE_EQ(result, 123);
    EXPECT_NE(error[0], '\0');
    EXPECT_EQ(descriptor.invoke(nullptr, 1, &result, nullptr, 0), CALC_PLUGIN_INVALID_ARGUMENT);
    EXPECT_EQ(descriptor.invoke(&argument, 1, nullptr, nullptr, 0), CALC_PLUGIN_INVALID_ARGUMENT);
    EXPECT_EQ(descriptor.invoke(&argument, 1, &result, nullptr, 1), CALC_PLUGIN_INVALID_ARGUMENT);
    argument = std::numeric_limits<double>::quiet_NaN();
    EXPECT_EQ(descriptor.invoke(&argument, 1, &result, error, sizeof(error)),
              CALC_PLUGIN_NUMERIC_ERROR);
    argument = 3.5;
    ASSERT_EQ(descriptor.invoke(&argument, 1, &result, error, sizeof(error)), CALC_PLUGIN_OK);
    EXPECT_DOUBLE_EQ(result, 3.5);
    EXPECT_EQ(error[0], '\0');
}

TEST(PluginSdk, TranslatesExceptionsAndBoundsErrorBuffers)
{
    constexpr auto         descriptor = calc::plugin::function<1, failing>("failing");
    const CalcPluginStatus expected[] = {CALC_PLUGIN_DOMAIN_ERROR, CALC_PLUGIN_INTERNAL_ERROR,
                                         CALC_PLUGIN_NUMERIC_ERROR};
    for (uint32_t index = 0; index < 3; ++index)
    {
        SCOPED_TRACE(index);
        const double argument = -static_cast<double>(index + 1);
        double       result   = 123;
        char         error[]  = {'x', 'z'};
        EXPECT_EQ(descriptor.invoke(&argument, 1, &result, error, 1), expected[index]);
        EXPECT_DOUBLE_EQ(result, 123);
        EXPECT_EQ(error[0], '\0');
        EXPECT_EQ(error[1], 'z');
        error[0] = 'x';
        EXPECT_EQ(descriptor.invoke(&argument, 1, &result, error, 0), expected[index]);
        EXPECT_EQ(error[0], 'x');
        EXPECT_EQ(descriptor.invoke(&argument, 1, &result, nullptr, 0), expected[index]);
    }
}

} // namespace
