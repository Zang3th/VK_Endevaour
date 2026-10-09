#pragma once

#include <cstdint>
#include <limits>

namespace Engine
{
    // Aliases

    using b8 = bool;
    using c8 = char8_t;

    using i8  = std::int8_t;
    using i16 = std::int16_t;
    using i32 = std::int32_t;
    using i64 = std::int64_t;

    using u8  = std::uint8_t;
    using u16 = std::uint16_t;
    using u32 = std::uint32_t;
    using u64 = std::uint64_t;

    using f32 = float;
    using f64 = double;

    using ull = unsigned long long;

    // Platform assumptions

    static_assert(sizeof(b8) == 1, "b8 must be 1 byte");
    static_assert(sizeof(f32) == 4 && std::numeric_limits<f32>::is_iec559, "f32 must be IEEE-754 binary32");
    static_assert(sizeof(f64) == 8 && std::numeric_limits<f64>::is_iec559, "f64 must be IEEE-754 binary64");
    static_assert(sizeof(ull) == 8, "ull must be 64 bit");
}
