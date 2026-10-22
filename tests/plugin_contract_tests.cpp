#include <calc/plugin_api.h>
#include <calc/plugin_sdk.h>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include <cstring>
#include <iostream>
#include <limits>
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

void check_invalid_metadata(CalcPluginApi metadata)
{
    CalcPluginApi output{};
    output.argument_count = 99;
    require(calc::plugin::publish(metadata, 1, sizeof(output), &output) ==
                    CALC_PLUGIN_INVALID_ARGUMENT &&
                output.argument_count == 99,
            "Invalid metadata must be rejected without changing the output");
}

void check_metadata(const CalcPluginApi& valid)
{
    auto invalid = valid;
    invalid.kind = 99;
    check_invalid_metadata(invalid);
    invalid             = valid;
    invalid.abi_version = 99;
    check_invalid_metadata(invalid);
    invalid             = valid;
    invalid.struct_size = 1;
    check_invalid_metadata(invalid);
    invalid        = valid;
    invalid.invoke = nullptr;
    check_invalid_metadata(invalid);
    invalid                = valid;
    invalid.argument_count = 0;
    check_invalid_metadata(invalid);
    invalid.argument_count = CALC_PLUGIN_MAX_ARGUMENTS + 1;
    check_invalid_metadata(invalid);
    invalid            = valid;
    invalid.precedence = 1;
    check_invalid_metadata(invalid);
    invalid               = valid;
    invalid.associativity = CALC_PLUGIN_ASSOC_LEFT;
    check_invalid_metadata(invalid);
    invalid = valid;
    std::memset(invalid.name, 'a', sizeof(invalid.name));
    check_invalid_metadata(invalid);
    for (const char* name : {"", "2sin", "bad name", "sin+", "^"})
    {
        invalid = valid;
        std::memcpy(invalid.name, name, std::strlen(name) + 1);
        check_invalid_metadata(invalid);
    }
    auto infix = valid;
    std::memcpy(infix.name, "^", 2);
    infix.kind           = CALC_PLUGIN_INFIX;
    infix.argument_count = 2;
    infix.precedence     = 4;
    infix.associativity  = CALC_PLUGIN_ASSOC_RIGHT;
    CalcPluginApi output{};
    require(calc::plugin::publish(infix, 1, sizeof(output), &output) == CALC_PLUGIN_OK &&
                output.argument_count == 2 && output.precedence == 4 &&
                output.associativity == CALC_PLUGIN_ASSOC_RIGHT,
            "Valid binary operator metadata must be accepted");
    invalid                = infix;
    invalid.argument_count = 1;
    check_invalid_metadata(invalid);
    invalid               = infix;
    invalid.associativity = CALC_PLUGIN_ASSOC_NONE;
    check_invalid_metadata(invalid);
    invalid            = infix;
    invalid.precedence = 0;
    check_invalid_metadata(invalid);
    invalid.precedence = 101;
    check_invalid_metadata(invalid);
    for (const char* name : {"+", "(", "ab", "7", ".", " "})
    {
        invalid = infix;
        std::memcpy(invalid.name, name, std::strlen(name) + 1);
        check_invalid_metadata(invalid);
    }
}

class Library
{
  public:
    explicit Library(const wchar_t* path) : handle_(LoadLibraryW(path))
    {
        require(handle_ != nullptr, "Cannot load test DLL");
    }

    ~Library()
    {
        FreeLibrary(handle_);
    }

    Library(const Library&)            = delete;
    Library& operator=(const Library&) = delete;

    CalcPluginGetApi entry() const
    {
        return reinterpret_cast<CalcPluginGetApi>(GetProcAddress(handle_, CALC_PLUGIN_ENTRY_NAME));
    }

  private:
    HMODULE handle_;
};

void check_errors(const wchar_t* path)
{
    Library    library(path);
    const auto entry = library.entry();
    require(entry != nullptr, "Probe DLL has no API entry");
    CalcPluginApi api{};
    require(entry(1, sizeof(api), &api) == CALC_PLUGIN_OK, "Cannot query probe metadata");
    require(std::strcmp(api.name, "probe") == 0, "Incorrect probe name");

    const CalcPluginStatus expected[] = {CALC_PLUGIN_DOMAIN_ERROR,   CALC_PLUGIN_NUMERIC_ERROR,
                                         CALC_PLUGIN_INTERNAL_ERROR, CALC_PLUGIN_INTERNAL_ERROR,
                                         CALC_PLUGIN_NUMERIC_ERROR,  CALC_PLUGIN_INVALID_ARGUMENT,
                                         CALC_PLUGIN_NUMERIC_ERROR,  CALC_PLUGIN_NUMERIC_ERROR};
    for (uint32_t index = 0; index < 8; ++index)
    {
        const double argument = -static_cast<double>(index + 1);
        double       result   = 123.0;
        char         error[64]{};
        require(api.invoke(&argument, 1, &result, error, sizeof(error)) == expected[index] &&
                    result == 123.0 && error[0] != '\0',
                "Exception or numeric error was not translated inside the DLL");
        char bounded[] = {'x', 'z'};
        require(api.invoke(&argument, 1, &result, bounded, 1) == expected[index] &&
                    bounded[0] == '\0' && bounded[1] == 'z',
                "One-byte error buffer must be terminated without overflow");
        bounded[0] = 'x';
        require(api.invoke(&argument, 1, &result, bounded, 0) == expected[index] &&
                    bounded[0] == 'x',
                "Zero-capacity buffer must not be written");
        require(api.invoke(&argument, 1, &result, nullptr, 0) == expected[index],
                "Errors must work without an error buffer");
    }
    double argument = std::numeric_limits<double>::quiet_NaN();
    double result   = 123.0;
    char   error[64]{};
    require(api.invoke(&argument, 1, &result, error, sizeof(error)) == CALC_PLUGIN_NUMERIC_ERROR &&
                result == 123.0,
            "Non-finite argument must be rejected");
    argument = 4.0;
    require(api.invoke(&argument, 1, &result, nullptr, 1) == CALC_PLUGIN_INVALID_ARGUMENT,
            "Nonzero capacity requires a buffer");
    require(api.invoke(&argument, 1, &result, error, sizeof(error)) == CALC_PLUGIN_OK &&
                result == 16.0 && error[0] == '\0',
            "DLL must remain usable after errors");
}

double sum(const double* arguments, uint32_t)
{
    return arguments[0] + arguments[1];
}

void check_infix_sdk()
{
    constexpr auto api = calc::plugin::infix<sum>("?", 1, CALC_PLUGIN_ASSOC_LEFT);
    require(calc::plugin::validation_error(api) == nullptr && api.kind == CALC_PLUGIN_INFIX &&
                api.argument_count == 2 && api.precedence == 1 &&
                api.associativity == CALC_PLUGIN_ASSOC_LEFT,
            "SDK must describe binary operators");
    const double arguments[] = {2, 3};
    double       result      = 0;
    require(api.invoke(arguments, 2, &result, nullptr, 0) == CALC_PLUGIN_OK && result == 5,
            "Binary callback must receive both arguments in order");
}

} // namespace

int wmain(int argc, wchar_t** argv)
{
    try
    {
        require(argc == 3, "Expected two DLL paths");
        Library    library(argv[1]);
        const auto entry = library.entry();
        require(entry != nullptr, "DLL must export calc_plugin_get_api");
        CalcPluginApi api{};
        require(entry(CALC_PLUGIN_ABI_VERSION, sizeof(api), &api) == CALC_PLUGIN_OK,
                "Cannot query operation metadata");
        require(api.abi_version == CALC_PLUGIN_ABI_VERSION && api.struct_size == sizeof(api),
                "Incorrect ABI metadata");
        require(std::strcmp(api.name, "identity") == 0,
                "Name must come from metadata, not filename");
        require(api.kind == CALC_PLUGIN_FUNCTION && api.argument_count == 1 &&
                    api.precedence == 0 && api.associativity == CALC_PLUGIN_ASSOC_NONE,
                "Incorrect function parsing rules");
        require(api.invoke != nullptr, "Missing operation callback");
        double argument    = 3.5;
        double result      = -100.0;
        char   message[64] = "stale error";
        require(api.invoke(&argument, 1, &result, message, sizeof(message)) == CALC_PLUGIN_OK &&
                    result == 3.5 && message[0] == '\0',
                "Incorrect successful invocation");
        require(entry(999, sizeof(api), &api) == CALC_PLUGIN_INCOMPATIBLE_ABI,
                "Unsupported ABI must be rejected");
        require(entry(1, sizeof(api) - 1, &api) == CALC_PLUGIN_INCOMPATIBLE_ABI,
                "Wrong structure size must be rejected");
        require(entry(1, sizeof(api), nullptr) == CALC_PLUGIN_INVALID_ARGUMENT,
                "Null metadata destination must be rejected");
        result = -100.0;
        require(api.invoke(&argument, 0, &result, message, sizeof(message)) ==
                        CALC_PLUGIN_INVALID_ARGUMENT &&
                    result == -100.0 && message[0] != '\0',
                "Invalid argument count must preserve the result and report an error");
        require(api.invoke(nullptr, 1, &result, nullptr, 0) == CALC_PLUGIN_INVALID_ARGUMENT,
                "Null arguments must be rejected without requiring an error buffer");
        require(api.invoke(&argument, 1, nullptr, message, sizeof(message)) ==
                    CALC_PLUGIN_INVALID_ARGUMENT,
                "Null result must be rejected");
        check_metadata(api);
        check_errors(argv[2]);
        check_infix_sdk();
        std::cout << "Plugin contract checks passed\n";
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
