#pragma once

#include <calc/plugin_api.h>

#include <filesystem>
#include <iosfwd>
#include <map>
#include <memory>
#include <string>
#include <string_view>

namespace calc
{

struct PluginOperation
{
    CalcPluginApi         api;
    std::shared_ptr<void> library;

    double evaluate(const double* arguments, uint32_t count) const;
};

class PluginRegistry
{
  public:
    std::shared_ptr<const PluginOperation> find(std::string_view name) const;

  private:
    std::map<std::string, std::shared_ptr<const PluginOperation>, std::less<>> operations_;

    friend PluginRegistry load_plugins(const std::filesystem::path&, std::ostream&);
};

PluginRegistry load_plugins(const std::filesystem::path& directory, std::ostream& diagnostics);
std::filesystem::path executable_directory();

} // namespace calc
