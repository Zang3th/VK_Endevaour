#pragma once

#include <filesystem>
#include <vector>

#include "Types.hpp"

namespace Engine
{
    class Utility
    {
    public:
        Utility() = delete;

        static std::vector<char> ReadFileAsBytes(const std::filesystem::path& path);
        static std::string       BytesToString(u64 bytes);
    };
}
