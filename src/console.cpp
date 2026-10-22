#include "console.h"

#include "evaluator.h"
#include "parser.h"

#include <algorithm>
#include <cctype>
#include <exception>
#include <iomanip>
#include <istream>
#include <limits>
#include <ostream>
#include <string>

namespace calc
{

int run_console(std::istream& input, std::ostream& output, std::ostream& errors,
                const PluginRegistry& plugins)
{
    int         exit_code = 0;
    std::string line;
    while (std::getline(input, line))
    {
        if (std::all_of(line.begin(), line.end(),
                        [](unsigned char character)
                        {
                            return std::isspace(character) != 0;
                        }))
        {
            continue;
        }
        try
        {
            const auto   expression = parse(line, plugins);
            const double result     = evaluate(expression);
            output << std::setprecision(std::numeric_limits<double>::max_digits10)
                   << (result == 0.0 ? 0.0 : result) << '\n';
        }
        catch (const std::exception& error)
        {
            errors << "Error: " << error.what() << '\n';
            exit_code = 1;
        }
    }
    if (!input.eof())
    {
        errors << "Error: Failed to read input\n";
        exit_code = 1;
    }
    return exit_code;
}

} // namespace calc
