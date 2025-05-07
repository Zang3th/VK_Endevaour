#pragma once

#include "Graphics/Mesh.hpp"

#include <vulkan/vulkan.hpp>

#include <filesystem>

namespace Engine
{
    class VulkanModel
    {
        public:
            VulkanModel(const vk::Device& device, const std::filesystem::path& path);

        private:
            Mesh m_Mesh;
    };
}
