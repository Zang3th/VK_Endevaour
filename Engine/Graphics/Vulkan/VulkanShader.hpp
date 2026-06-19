#pragma once

#include <vulkan/vulkan.hpp>

#include <filesystem>

namespace Engine::Graphics
{
    class VulkanShader
    {
    public:
        VulkanShader(const vk::Device& device, vk::ShaderStageFlagBits stage, const std::filesystem::path& path);
        ~VulkanShader();

        VulkanShader(const VulkanShader&)            = delete;
        VulkanShader& operator=(const VulkanShader&) = delete;

        [[nodiscard]] vk::PipelineShaderStageCreateInfo GetPipelineShaderStageCreateInfo() const;

    private:
        void CreateShaderModule(std::vector<char>&& code);

        vk::Device              m_Device = nullptr;
        vk::ShaderModule        m_Module = nullptr;
        vk::ShaderStageFlagBits m_Stage;
        std::string             m_StageString;
    };
}
