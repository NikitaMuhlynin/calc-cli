#include "plugin_registry.h"

#include <calc/plugin_validation.h>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include <algorithm>
#include <cmath>
#include <cwctype>
#include <ostream>
#include <stdexcept>
#include <system_error>
#include <vector>

namespace calc
{

double PluginOperation::evaluate(const double* arguments, uint32_t count) const
{
    if (arguments == nullptr || count != api.argument_count)
    {
        throw std::invalid_argument("Invalid plugin argument count");
    }
    char       error[512]{};
    double     result        = 0.0;
    const auto status        = api.invoke(arguments, count, &result, error, sizeof(error));
    error[sizeof(error) - 1] = '\0';
    if (status != CALC_PLUGIN_OK)
    {
        throw std::runtime_error("Plugin '" + std::string(api.name) + "' (status " +
                                 std::to_string(status) + "): " + error);
    }
    if (!std::isfinite(result))
    {
        throw std::overflow_error("Plugin '" + std::string(api.name) + "': non-finite result");
    }
    return result;
}

std::shared_ptr<const PluginOperation> PluginRegistry::find(std::string_view name) const
{
    const auto found = operations_.find(name);
    return found == operations_.end() ? nullptr : found->second;
}

PluginRegistry load_plugins(const std::filesystem::path& directory, std::ostream& diagnostics)
{
    PluginRegistry                      registry;
    std::error_code                     error;
    std::filesystem::directory_iterator iterator(directory, error);
    if (error)
    {
        diagnostics << "Warning: Cannot scan plugins directory '" << directory.u8string()
                    << "': " << error.message() << '\n';
        return registry;
    }
    std::vector<std::filesystem::path>        files;
    const std::filesystem::directory_iterator end;
    while (iterator != end)
    {
        const auto path      = iterator->path();
        auto       extension = path.extension().wstring();
        std::transform(extension.begin(), extension.end(), extension.begin(),
                       [](wchar_t character)
                       {
                           return static_cast<wchar_t>(std::towlower(character));
                       });
        std::error_code type_error;
        if (extension == L".dll" && iterator->is_regular_file(type_error))
        {
            files.push_back(path);
        }
        iterator.increment(error);
        if (error)
        {
            diagnostics << "Warning: Cannot finish scanning plugins: " << error.message() << '\n';
            break;
        }
    }
    std::sort(files.begin(), files.end());
    for (const auto& path : files)
    {
        try
        {
            const auto absolute_path = std::filesystem::absolute(path);
            const auto module =
                LoadLibraryExW(absolute_path.c_str(), nullptr,
                               LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
            if (module == nullptr)
            {
                throw std::system_error(static_cast<int>(GetLastError()), std::system_category(),
                                        "Cannot load DLL");
            }
            const std::shared_ptr<void> library(module,
                                                [](void* handle)
                                                {
                                                    FreeLibrary(static_cast<HMODULE>(handle));
                                                });
            const auto                  entry =
                reinterpret_cast<CalcPluginGetApi>(GetProcAddress(module, CALC_PLUGIN_ENTRY_NAME));
            if (entry == nullptr)
            {
                throw std::runtime_error("Missing calc_plugin_get_api export");
            }
            CalcPluginApi api{};
            const auto    status = entry(CALC_PLUGIN_ABI_VERSION, sizeof(api), &api);
            if (status != CALC_PLUGIN_OK)
            {
                throw std::runtime_error("API query failed with status " + std::to_string(status));
            }
            if (const auto reason = plugin::validation_error(api))
            {
                throw std::runtime_error(reason);
            }
            if (registry.operations_.find(api.name) != registry.operations_.end())
            {
                throw std::runtime_error("Duplicate operation '" + std::string(api.name) + "'");
            }
            registry.operations_.emplace(
                api.name, std::make_shared<const PluginOperation>(PluginOperation{api, library}));
        }
        catch (const std::exception& exception)
        {
            diagnostics << "Warning: Skipping plugin '" << path.u8string()
                        << "': " << exception.what() << '\n';
        }
    }
    if (registry.operations_.empty())
    {
        diagnostics << "Warning: No valid DLL plugins found; basic arithmetic is available\n";
    }
    return registry;
}

std::filesystem::path executable_directory()
{
    std::vector<wchar_t> buffer(260);
    for (;;)
    {
        const auto length =
            GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
        if (length == 0)
        {
            throw std::system_error(static_cast<int>(GetLastError()), std::system_category(),
                                    "Cannot locate executable");
        }
        if (length < buffer.size())
        {
            return std::filesystem::path(std::wstring(buffer.data(), length)).parent_path();
        }
        buffer.resize(buffer.size() * 2);
    }
}

} // namespace calc
