#include "Utility.hpp"
#include "Debug/Log.hpp"

#include <fstream>
#include <array>

namespace Engine
{
    std::vector<char> Utility::ReadFileAsBytes(const std::filesystem::path& path)
    {
        // Open file as binary and immediately move to the end
        std::ifstream file(path, std::ios::ate | std::ios::binary);
        ASSERT(file.is_open(), "Can't open file: {}", path.string());

        // Get the file size, set pointer back to the beginning and read into buffer
        const std::streamsize fileSize = file.tellg();
        std::vector<char> buffer(fileSize);
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

        std::array<char, 16> buffer;

        if (bytes >= GB)
        {
            snprintf(buffer.data(), buffer.size(), "%.2f GB", (f64)bytes / (f64)GB);
        }
        else if (bytes >= MB)
        {
            snprintf(buffer.data(), buffer.size(), "%.2f MB", (f64)bytes / (f64)MB);
        }
        else if (bytes >= KB)
        {
            snprintf(buffer.data(), buffer.size(), "%.2f KB", (f64)bytes / (f64)KB);
        }
        else
        {
            snprintf(buffer.data(), buffer.size(), "%llu bytes", bytes);
        }

        return buffer.data();
    }
}
