#include "VulkanShader.hpp"
#include "VulkanAssert.hpp"

#include "Core/Utility.hpp"
#include "Core/Types.hpp"
#include "Debug/Log.hpp"

namespace Engine
{
    // ----- Public -----

    VulkanShader::VulkanShader(const vk::Device& device, vk::ShaderStageFlagBits stage, const std::filesystem::path& path)
        : m_Device(device), m_Stage(stage)
    {
        std::vector<char> code = Utility::ReadFileAsBytes(path);
        CreateShaderModule(std::move(code));
    }

    VulkanShader::~VulkanShader()
    {
        LOG_INFO("VulkanShader::Destructor() ...");
        m_Device.destroyShaderModule(m_Module);
    }

    [[nodiscard]] vk::PipelineShaderStageCreateInfo VulkanShader::GetPipelineShaderStageCreateInfo()
    {
        return
        {
            .stage = m_Stage,
            .module = m_Module,
            .pName = "main"
        };
    }

    // ----- Private -----

    void VulkanShader::CreateShaderModule(std::vector<char>&& code)
    {
        vk::ShaderModuleCreateInfo shaderCreateInfo
        {
            .codeSize = code.size(),
            .pCode    = (u32*)code.data()
        };

        VK_VERIFY(m_Device.createShaderModule(&shaderCreateInfo, nullptr, &m_Module));
        LOG_INFO("Created shader module ...");
    }
}
