#include "Core/Debug/Log.hpp"

int main()
{
    LOG_INFO("This is an error message with value: {}", 42);
    LOG_WARN("This is an error message with value: {}", 43);
    LOG_ERROR("This is an error message with value: {}", 44);

    return 0;
}
