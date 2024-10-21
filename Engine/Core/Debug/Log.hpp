#pragma once

#include "Vendor/fmt/include/fmt/core.h"
#include "Vendor/fmt/include/fmt/chrono.h"
#include "Vendor/fmt/include/fmt/color.h"

#include <cstdio>

namespace Engine
{
    // Not ideal to route everything through stderr but unifies colored output from validation layers for now

    #ifdef _WIN32
        #define GET_CURRENT_TIME_WITH_MILLIS() \
            auto now = std::chrono::system_clock::now(); \
            auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000; \
            std::time_t tt = std::chrono::system_clock::to_time_t(now); \
            std::tm localTime{}; \
            localtime_s(&localTime, &tt); // Thread-safe on Windows ?
    #else
        #define GET_CURRENT_TIME_WITH_MILLIS() \
            auto now = std::chrono::system_clock::now(); \
            auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000; \
            std::time_t tt = std::chrono::system_clock::to_time_t(now); \
            std::tm localTime{}; \
            localtime_r(&tt, &localTime);
    #endif

    #define LOG_VERBOSE(msg, ...) do { \
        GET_CURRENT_TIME_WITH_MILLIS(); \
        fmt::print(stderr, fg(fmt::color::white) | fmt::emphasis::bold, \
                   "{:%H:%M:%S}.{:03d} [VERBOSE] " msg "\n", localTime, static_cast<int>(millis.count()) \
                   __VA_OPT__(,) __VA_ARGS__); \
    } while(0)

    #define LOG_INFO(msg, ...) do { \
        GET_CURRENT_TIME_WITH_MILLIS(); \
        fmt::print(stderr, fg(fmt::color::green) | fmt::emphasis::bold, \
                   "{:%H:%M:%S}.{:03d} [INFO] " msg "\n", localTime, static_cast<int>(millis.count()) \
                   __VA_OPT__(,) __VA_ARGS__); \
    } while(0)


    #define LOG_WARN(msg, ...) do { \
        GET_CURRENT_TIME_WITH_MILLIS(); \
        fmt::print(stderr, fg(fmt::color::yellow) | fmt::emphasis::bold, \
                   "{:%H:%M:%S}.{:03d} [WARN] " msg "\n", localTime, static_cast<int>(millis.count()) \
                   __VA_OPT__(,) __VA_ARGS__); \
    } while(0)

    #define LOG_ERROR(msg, ...) do { \
        GET_CURRENT_TIME_WITH_MILLIS(); \
        fmt::print(stderr, fg(fmt::color::crimson) | fmt::emphasis::bold, \
                   "{:%H:%M:%S}.{:03d} [ERROR] " msg "\n", localTime, static_cast<int>(millis.count()) \
                   __VA_OPT__(,) __VA_ARGS__); \
    } while(0)

    #define LOG_ASSERT(msg, ...) do { \
        GET_CURRENT_TIME_WITH_MILLIS(); \
        fmt::print(stderr, fg(fmt::color::crimson) | fmt::emphasis::bold, \
                   "{:%H:%M:%S}.{:03d} [ASSERT] " msg "\n", localTime, static_cast<int>(millis.count()) \
                   __VA_OPT__(,) __VA_ARGS__); \
    } while(0)

    #define ASSERT(condition, ...) { if(!condition) { LOG_ASSERT(__VA_ARGS__); __builtin_debugtrap(); }}
}
