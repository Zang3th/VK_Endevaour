#pragma once

#include "Types.hpp"

#include <vector>
#include <filesystem>

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
