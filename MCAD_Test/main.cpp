#include "ExampleRunner.h"

#include <cstdlib>
#include <exception>
#include <iostream>

int main()
{
    try
    {
        // ExampleRunner::runBasicSubtract();
        // ExampleRunner::runUnionWithThreeHoles();
        // ExampleRunner::runUntrackedBaseWithCut();
        ExampleRunner::runTripleUnionWithThroughHole();
    }
    catch (const std::exception& exception)
    {
        std::cerr << "Error: " << exception.what() << '\n';
        return EXIT_FAILURE;
    }
    catch (...)
    {
        std::cerr << "Unknown error\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
