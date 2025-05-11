#include "Utility.hpp"
#include "Debug/Log.hpp"

#include <fstream>

namespace Engine
{
    std::vector<char> Utility::ReadFileAsBytes(const std::filesystem::path& path)
    {
        // Open file as binary and immediately move to the end
        std::ifstream file(path, std::ios::ate | std::ios::binary);
        ASSERT(file.is_open(), "Can't open file: {}", path.string());

        // Get the file size, set pointer back to the beginning and read into buffer
        std::streamsize fileSize = file.tellg();
        std::vector<char> buffer(fileSize);
        file.seekg(0);
        file.read(buffer.data(), fileSize);
        file.close();
        LOG_INFO("Read in file '{}' ... ({})", path.string(), BytesToString(fileSize));

        return buffer;
    }

    std::string Utility::BytesToString(u64 bytes)
    {
        constexpr u64 GB = 1024 * 1024 * 1024;
        constexpr u64 MB = 1024 * 1024;
        constexpr u64 KB = 1024;

        char buffer[32 + 1] {};

        if (bytes >= GB)
        {
            snprintf(buffer, 32, "%.2f GB", (f32)bytes / (f32)GB);
        }
        else if (bytes >= MB)
        {
            snprintf(buffer, 32, "%.2f MB", (f32)bytes / (f32)MB);
        }
        else if (bytes >= KB)
        {
            snprintf(buffer, 32, "%.2f KB", (f32)bytes / (f32)KB);
        }
        else
        {
            snprintf(buffer, 32, "%.2f bytes", (f32)bytes);
        }

        return buffer;
    }
}
