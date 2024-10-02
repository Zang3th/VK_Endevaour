#pragma once

#include "Vendor/fmt/include/fmt/core.h"
#include "Vendor/fmt/include/fmt/chrono.h"
#include "Vendor/fmt/include/fmt/color.h"

#include <cstdio>

namespace Engine
{
    // Not ideal to route everything through stderr but unifies colored output from validation layers for now

    #define LOG_VERBOSE(msg, ...) do { \
        fmt::print(stderr, fg(fmt::color::white) | fmt::emphasis::bold, \
                   "{:%M:%S} [VERBOSE] " msg "\n", std::chrono::system_clock::now() \
                   __VA_OPT__(,) __VA_ARGS__); \
    } while(0)

    #define LOG_INFO(msg, ...) do { \
        fmt::print(stderr, fg(fmt::color::green) | fmt::emphasis::bold, \
                   "{:%M:%S} [INFO] " msg "\n", std::chrono::system_clock::now() \
                   __VA_OPT__(,) __VA_ARGS__); \
    } while(0)

    #define LOG_WARN(msg, ...) do { \
        fmt::print(stderr, fg(fmt::color::yellow) | fmt::emphasis::bold, \
                   "{:%M:%S} [WARN] " msg "\n", std::chrono::system_clock::now() \
                   __VA_OPT__(,) __VA_ARGS__); \
    } while(0)

    #define LOG_ERROR(msg, ...) do { \
        fmt::print(stderr, fg(fmt::color::crimson) | fmt::emphasis::bold, \
                   "{:%M:%S} [ERROR] " msg "\n", std::chrono::system_clock::now() \
                   __VA_OPT__(,) __VA_ARGS__); \
    } while(0)

    #define LOG_ASSERT(msg, ...) do { \
        fmt::print(stderr, fg(fmt::color::crimson) | fmt::emphasis::bold, \
                   "{:%M:%S} [ASSERT] " msg "\n", std::chrono::system_clock::now() \
                   __VA_OPT__(,) __VA_ARGS__); \
    } while(0)

    #define ASSERT(condition, ...) { if(!condition) { LOG_ASSERT(__VA_ARGS__); __builtin_debugtrap(); }}
}
