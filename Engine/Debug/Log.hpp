#pragma once

#include "Vendor/fmt/include/fmt/core.h"
#include "Vendor/fmt/include/fmt/color.h"

#define LOG_VERBOSE(msg, ...) fmt::print(stdout, "[VERBOSE] " msg "\n" __VA_OPT__(,) __VA_ARGS__)

#define LOG_INFO(msg, ...) fmt::print(stdout, fg(fmt::color::green), "[INFO] " msg "\n" __VA_OPT__(,) __VA_ARGS__)

#define LOG_WARN(msg, ...) fmt::print(stdout, fg(fmt::color::yellow), "[WARN] " msg "\n" __VA_OPT__(,) __VA_ARGS__)

#define LOG_ERROR(msg, ...) fmt::print(stdout, fg(fmt::color::crimson), "[ERROR] " msg "\n" __VA_OPT__(,) __VA_ARGS__)

#define LOG_ASSERT(msg, ...) fmt::print(stdout, fg(fmt::color::purple) | fmt::emphasis::bold, "[ASSERT] " msg "\n" __VA_OPT__(,) __VA_ARGS__)

#define ASSERT(condition, ...) { if(!(condition)) { LOG_ASSERT(__VA_ARGS__); __builtin_trap(); }}
