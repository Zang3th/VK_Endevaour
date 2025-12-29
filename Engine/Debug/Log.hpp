#pragma once

#include "Vendor/fmt/include/fmt/core.h"
#include "Vendor/fmt/include/fmt/chrono.h"
#include "Vendor/fmt/include/fmt/color.h"

// Not ideal to route everything through stderr, but this unifies colored output from validation layers for every tested terminal.

#define LOG_VERBOSE(msg, ...) fmt::print(stderr, "[VERBOSE] " msg "\n" __VA_OPT__(,) __VA_ARGS__)

#define LOG_INFO(msg, ...) fmt::print(stderr, "[INFO] " msg "\n" __VA_OPT__(,) __VA_ARGS__)

#define LOG_WARN(msg, ...) fmt::print(stderr, "[WARN] " msg "\n" __VA_OPT__(,) __VA_ARGS__)

#define LOG_ERROR(msg, ...) fmt::print(stderr, "[ERROR] " msg "\n" __VA_OPT__(,) __VA_ARGS__)

#define LOG_ASSERT(msg, ...) fmt::print(stderr, "[ASSERT] " msg "\n" __VA_OPT__(,) __VA_ARGS__)

#define ASSERT(condition, ...) { if(!(condition)) { LOG_ASSERT(__VA_ARGS__); __builtin_trap(); }}
