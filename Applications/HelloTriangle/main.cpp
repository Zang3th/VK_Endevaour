#include <cstdlib>

#include "HelloTriangle.hpp"

int main()
{
    HelloTriangle app;

    if(app.Run() != EXIT_SUCCESS)
    {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
