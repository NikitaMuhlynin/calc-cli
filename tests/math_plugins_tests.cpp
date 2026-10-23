#include "evaluator.h"
#include "parser.h"
#include "plugin_registry.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace
{

struct Case
{
    const char* expression;
    double      expected;
};

} // namespace

int wmain(int argc, wchar_t** argv)
{
    try
    {
        if (argc != 2)
        {
            throw std::runtime_error("Expected a plugins directory");
        }
        std::ostringstream diagnostics;
        const auto         registry = calc::load_plugins(argv[1], diagnostics);
        if (!diagnostics.str().empty())
        {
            throw std::runtime_error(diagnostics.str());
        }
        const Case cases[] = {{"sin(30)", 0.5},
                              {"sin(-30)", -0.5},
                              {"sin(45)", 0.7071067811865476},
                              {"sin(360000030)", 0.5},
                              {"2^0.5", 1.4142135623730951},
                              {"4^-0.5", 0.5},
                              {"2^sin(90)+sin(30)", 2.5}};
        for (const auto& item : cases)
        {
            const double actual = calc::evaluate(calc::parse(item.expression, registry));
            if (!std::isfinite(actual) ||
                std::abs(actual - item.expected) > 1e-12 * std::max(1.0, std::abs(item.expected)))
            {
                throw std::runtime_error(std::string("Incorrect result: ") + item.expression);
            }
        }
        std::cout << "Math plugin checks passed\n";
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
