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
        LOG_INFO("Read in file '{}' ... ({} bytes)", path.string(), fileSize);

        return buffer;
    }
}
