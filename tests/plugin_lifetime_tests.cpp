#include "evaluator.h"
#include "parser.h"
#include "plugin_registry.h"

#include <iostream>
#include <sstream>
#include <stdexcept>

int wmain(int argc, wchar_t** argv)
{
    try
    {
        if (argc != 2)
        {
            throw std::runtime_error("Expected a plugins directory");
        }
        calc::Expression   expression;
        std::ostringstream diagnostics;
        {
            const auto registry = calc::load_plugins(argv[1], diagnostics);
            expression          = calc::parse("identity(42)", registry);
        }
        if (calc::evaluate(expression) != 42 || !diagnostics.str().empty())
        {
            throw std::runtime_error("Expression must retain its DLL after registry destruction");
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
