#include "console.h"

#include <iostream>

int main()
{
    return calc::run_console(std::cin, std::cout, std::cerr);
}
