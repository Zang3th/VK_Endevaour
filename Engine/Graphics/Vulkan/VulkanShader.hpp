#pragma once

#include <vulkan/vulkan.hpp>

#include <filesystem>

namespace Engine
{
    class VulkanShader
    {
        public:
            VulkanShader(const vk::Device& device, vk::ShaderStageFlagBits stage, const std::filesystem::path& path);
            ~VulkanShader();

            [[nodiscard]] vk::PipelineShaderStageCreateInfo GetPipelineShaderStageCreateInfo();

        private:
            void CreateShaderModule(std::vector<char>&& code);

            vk::Device              m_Device = nullptr;
            vk::ShaderModule        m_Module = nullptr;
            vk::ShaderStageFlagBits m_Stage;
    };
}
