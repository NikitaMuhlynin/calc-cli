#include "console.h"
#include "plugin_registry.h"

#include <iostream>

int main()
{
    try
    {
        const auto plugins =
            calc::load_plugins(calc::executable_directory() / "plugins", std::cerr);
        return calc::run_console(std::cin, std::cout, std::cerr, plugins);
    }
    catch (const std::exception& error)
    {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }
}
