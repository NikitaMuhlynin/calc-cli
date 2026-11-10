#include "evaluator.h"
#include "parser.h"
#include "plugin_registry.h"

#include <gtest/gtest.h>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <filesystem>
#include <sstream>

namespace
{

bool is_loaded(const std::filesystem::path& path)
{
    // Observes the module without incrementing its reference count.
    return GetModuleHandleW(path.c_str()) != nullptr;
}

TEST(PluginLifetime, ReleasesRejectedLibrariesAndRetainsUsedOperations)
{
    const auto directory = std::filesystem::u8path(CALC_TEST_WORK_DIRECTORY) / "plugins";
    const auto fixtures  = std::filesystem::u8path(CALC_TEST_FIXTURE_DIRECTORY);
    const auto identity  = directory / "a_identity.dll";
    const auto source    = calc::executable_directory() / "plugins" / "plugin_stub.dll";
    const std::filesystem::path rejected[] = {
        directory / "b_duplicate.dll", directory / "c_no_entry.dll",
        directory / "d_bad_metadata.dll", directory / "e_refused.dll"};
    std::filesystem::create_directories(directory);
    const auto overwrite = std::filesystem::copy_options::overwrite_existing;
    std::filesystem::copy_file(source, identity, overwrite);
    std::filesystem::copy_file(source, rejected[0], overwrite);
    std::filesystem::copy_file(fixtures / "plugin_no_entry.dll", rejected[1], overwrite);
    std::filesystem::copy_file(fixtures / "plugin_bad_metadata.dll", rejected[2], overwrite);
    std::filesystem::copy_file(fixtures / "plugin_refused.dll", rejected[3], overwrite);

    for (int iteration = 0; iteration < 3; ++iteration)
    {
        SCOPED_TRACE(iteration);
        ASSERT_FALSE(is_loaded(identity));
        calc::Expression   expression;
        std::ostringstream diagnostics;
        {
            const auto registry = calc::load_plugins(directory, diagnostics);
            ASSERT_TRUE(is_loaded(identity));
            for (const auto& path : rejected)
            {
                EXPECT_FALSE(is_loaded(path)) << path;
            }
            for (const char* reason : {"Duplicate operation", "Missing calc_plugin_get_api",
                                       "Incompatible plugin ABI", "API query failed with status 5"})
            {
                EXPECT_NE(diagnostics.str().find(reason), std::string::npos) << reason;
            }
            EXPECT_THROW(calc::parse("identity(1,2)", registry), std::invalid_argument);
            expression = calc::parse("identity(42)", registry);
        }
        ASSERT_TRUE(is_loaded(identity));
        EXPECT_DOUBLE_EQ(calc::evaluate(expression), 42);
        expression.clear();
        EXPECT_FALSE(is_loaded(identity));
    }
}

} // namespace
