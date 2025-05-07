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

            [[nodiscard]] const vk::ShaderModule& GetModule() const { return m_ShaderModule; }
            [[nodiscard]] vk::ShaderStageFlagBits GetStage()  const { return m_Stage;        }

        private:
            void CreateShaderModule(std::vector<char>&& code);

            vk::Device              m_Device       = nullptr;
            vk::ShaderModule        m_ShaderModule = nullptr;
            vk::ShaderStageFlagBits m_Stage;
    };
}
