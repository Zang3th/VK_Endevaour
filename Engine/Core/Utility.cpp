#include "Utility.hpp"

#include "Debug/Log.hpp"

#include <fstream>

namespace Engine::Core
{
    std::vector<char> Utility::ReadFileAsBytes(const std::filesystem::path& path)
    {
        // Open file as binary and immediately move to the end
        std::ifstream file(path, std::ios::ate | std::ios::binary);
        ASSERT(file.is_open(), "Can't open file: {}", path.string());

        // Get the file size, set pointer back to the beginning and read into buffer
        const std::streamsize fileSize = file.tellg();
        std::vector<char>     buffer(fileSize);
        file.seekg(0);
        file.read(buffer.data(), fileSize);
        file.close();
        LOG_INFO("Read in file '{}' ... ({})", path.string(), BytesToString(fileSize));

        return buffer;
    }

    std::string Utility::BytesToString(u64 bytes)
    {
        constexpr u64 GB = 1024ull * 1024 * 1024;
        constexpr u64 MB = 1024ull * 1024;
        constexpr u64 KB = 1024ull;

        if (bytes >= GB)
        {
            return fmt::format("{:.2f} GB", (f64)bytes / (f64)GB);
        }
        if (bytes >= MB)
        {
            return fmt::format("{:.2f} MB", (f64)bytes / (f64)MB);
        }
        if (bytes >= KB)
        {
            return fmt::format("{:.2f} KB", (f64)bytes / (f64)KB);
        }

        return fmt::format("{} bytes", bytes);
    }

    std::string Utility::MillisecondsToString(f64 ms)
    {
        constexpr f64 Second = 1000.0;
        constexpr f64 Minute = 60.0 * Second;

        if (ms >= Minute)
        {
            return fmt::format("{:.2f} min", ms / Minute);
        }
        if (ms >= Second)
        {
            return fmt::format("{:.2f} sec", ms / Second);
        }

        return fmt::format("{:.2f} ms", ms);
    }

    std::string Utility::FPSToString(f64 fps)
    {
        return fmt::format("{:.2f} FPS", fps);
    }
}
