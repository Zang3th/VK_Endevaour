#pragma once

#include "Core/Types.hpp"

#include "Vendor/glm/vec3.hpp"

#include <filesystem>
#include <vector>

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

        [[nodiscard]] static glm::vec3 GetRandomVec3();
    };
}
