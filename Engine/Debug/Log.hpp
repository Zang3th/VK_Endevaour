#pragma once

#include "Vendor/fmt/include/fmt/color.h"
#include "Vendor/fmt/include/fmt/core.h"

#include <source_location>

#define LOG_LOCATION() std::source_location::current().file_name(), std::source_location::current().line()

#define LOG_VERBOSE(msg, ...) fmt::print(stdout, "[VERBOSE] " msg "\n" __VA_OPT__(, ) __VA_ARGS__)

#define LOG_PERF(msg, ...) fmt::print(stdout, fg(fmt::color::aqua), "[PERF] " msg "\n" __VA_OPT__(, ) __VA_ARGS__)

#define LOG_INFO(msg, ...) fmt::print(stdout, fg(fmt::color::green), "[INFO] " msg "\n" __VA_OPT__(, ) __VA_ARGS__)

#define LOG_WARN(msg, ...) fmt::print(stdout, fg(fmt::color::yellow), "[WARN] " msg "\n" __VA_OPT__(, ) __VA_ARGS__)

#define LOG_ERROR(msg, ...)                                                                                            \
    do                                                                                                                 \
    {                                                                                                                  \
        fmt::print(                                                                                                    \
            stdout, fg(fmt::color::crimson) | fmt::emphasis::bold, "\n[ERROR] " msg "\n" __VA_OPT__(, __VA_ARGS__));   \
                                                                                                                       \
        fmt::print(stdout, fg(fmt::color::crimson), "     -> {}:{}\n", LOG_LOCATION());                                \
    } while (false)

#define ASSERT(condition, msg, ...)                                                                                    \
    do                                                                                                                 \
    {                                                                                                                  \
        if (!(condition))                                                                                              \
        {                                                                                                              \
            fmt::print(stdout,                                                                                         \
                       fg(fmt::color::purple) | fmt::emphasis::bold,                                                   \
                       "\n[ASSERT] " msg "\n" __VA_OPT__(, __VA_ARGS__));                                              \
                                                                                                                       \
            fmt::print(stdout, fg(fmt::color::purple), "      -> {}:{}\n", LOG_LOCATION());                            \
            __builtin_trap();                                                                                          \
        }                                                                                                              \
    } while (false)

#define LOG_HEADER(title, color)                                                                                       \
    do                                                                                                                 \
    {                                                                                                                  \
        fmt::print(stdout, fg(color) | fmt::emphasis::bold, "#################################\n");                    \
                                                                                                                       \
        fmt::print(stdout, fg(color) | fmt::emphasis::bold, "##### {:<21} #####\n", title);                            \
                                                                                                                       \
        fmt::print(stdout, fg(color) | fmt::emphasis::bold, "#################################\n");                    \
    } while (false)
