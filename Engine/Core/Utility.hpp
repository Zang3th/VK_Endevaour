#pragma once

#include <filesystem>
#include <vector>

#include "Types.hpp"

namespace Engine::Core
{
    class Utility
    {
    public:
        Utility() = delete;

        [[nodiscard]] static std::vector<char> ReadFileAsBytes(const std::filesystem::path& path);
        [[nodiscard]] static std::string       BytesToString(u64 bytes);
        [[nodiscard]] static std::string       MillisecondsToString(f64 ms);
        [[nodiscard]] static std::string       FPSToString(f64 fps);
    };
}
