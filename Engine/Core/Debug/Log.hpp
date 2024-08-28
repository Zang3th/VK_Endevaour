#pragma once

#include "Vendor/fmt/include/fmt/core.h"
#include "Vendor/fmt/include/fmt/chrono.h"
#include "Vendor/fmt/include/fmt/color.h"

namespace Engine
{
    #define LOG_INFO(msg, ...) do { \
        fmt::print(fg(fmt::color::green) | fmt::emphasis::bold, \
                   "{:%M:%S} [INFO]  " msg "\n", std::chrono::system_clock::now() \
                   __VA_OPT__(,) __VA_ARGS__); \
    } while(0)

    #define LOG_WARN(msg, ...) do { \
        fmt::print(fg(fmt::color::yellow) | fmt::emphasis::bold, \
                   "{:%M:%S} [WARN]  " msg "\n", std::chrono::system_clock::now() \
                   __VA_OPT__(,) __VA_ARGS__); \
    } while(0)

    #define LOG_ERROR(msg, ...) do { \
        fmt::print(fg(fmt::color::crimson) | fmt::emphasis::bold, \
                   "{:%M:%S} [ERROR] " msg "\n", std::chrono::system_clock::now() \
                   __VA_OPT__(,) __VA_ARGS__); \
    } while(0)
}
