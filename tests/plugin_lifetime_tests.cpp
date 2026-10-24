#include "evaluator.h"
#include "parser.h"
#include "plugin_registry.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include <filesystem>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace
{

void require(bool condition, const char* message)
{
    if (!condition)
    {
        throw std::runtime_error(message);
    }
}

bool is_loaded(const std::filesystem::path& path)
{
    // GetModuleHandleW observes the module without incrementing its reference count.
    return GetModuleHandleW(path.c_str()) != nullptr;
}

} // namespace

int wmain(int argc, wchar_t** argv)
{
    try
    {
        if (argc != 6)
        {
            throw std::runtime_error(
                "Expected identity, working directory and three rejected DLLs");
        }
        const auto                  directory  = std::filesystem::absolute(argv[2]) / "plugins";
        const auto                  identity   = directory / "a_identity.dll";
        const std::filesystem::path rejected[] = {
            directory / "b_duplicate.dll", directory / "c_no_entry.dll",
            directory / "d_bad_metadata.dll", directory / "e_refused.dll"};
        std::filesystem::create_directories(directory);
        const auto overwrite = std::filesystem::copy_options::overwrite_existing;
        std::filesystem::copy_file(argv[1], identity, overwrite);
        std::filesystem::copy_file(argv[1], rejected[0], overwrite);
        for (int index = 1; index < 4; ++index)
        {
            std::filesystem::copy_file(argv[index + 2], rejected[index], overwrite);
        }
        for (int iteration = 0; iteration < 3; ++iteration)
        {
            require(!is_loaded(identity), "Previous iteration leaked the valid DLL");
            calc::Expression   expression;
            std::ostringstream diagnostics;
            {
                const auto registry = calc::load_plugins(directory, diagnostics);
                require(is_loaded(identity), "Registered DLL must remain loaded");
                for (const auto& path : rejected)
                {
                    require(!is_loaded(path),
                            "Rejected or duplicate DLL must be unloaded immediately");
                }
                for (const char* reason :
                     {"Duplicate operation", "Missing calc_plugin_get_api",
                      "Incompatible plugin ABI", "API query failed with status 5"})
                {
                    require(diagnostics.str().find(reason) != std::string::npos,
                            "Missing rejected DLL diagnostic");
                }
                bool parse_failed = false;
                try
                {
                    calc::parse("identity(1,2)", registry);
                }
                catch (const std::invalid_argument&)
                {
                    parse_failed = true;
                }
                require(parse_failed, "Invalid expression must throw");
                expression = calc::parse("identity(42)", registry);
            }
            require(is_loaded(identity),
                    "Expression must retain its DLL after registry destruction");
            require(calc::evaluate(expression) == 42, "Retained DLL must remain callable");
            expression.clear();
            require(!is_loaded(identity), "Last expression must release the DLL");
        }
        std::cout << "Plugin lifetime check passed\n";
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
