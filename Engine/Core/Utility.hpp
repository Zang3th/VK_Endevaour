#pragma once

#include <vector>
#include <filesystem>

namespace Engine
{
    class Utility
    {
        public:
            Utility() = delete;
            static std::vector<char> ReadFileAsBytes(const std::filesystem::path& path);
    };
}
