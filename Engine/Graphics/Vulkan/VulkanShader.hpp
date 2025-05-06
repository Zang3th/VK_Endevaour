#pragma once

#include <vulkan/vulkan.hpp>

#include <filesystem>

namespace Engine
{
    class VulkanShader
    {
        public:
            VulkanShader(vk::ShaderStageFlagBits stage, const std::filesystem::path& path);

        private:
    };
}
