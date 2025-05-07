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
        m_Device.destroyShaderModule(m_ShaderModule);
    }

    // ----- Private -----

    void VulkanShader::CreateShaderModule(std::vector<char>&& code)
    {
        vk::ShaderModuleCreateInfo shaderCreateInfo
        {
            .codeSize = code.size(),
            .pCode    = (u32*)code.data()
        };

        VK_VERIFY(m_Device.createShaderModule(&shaderCreateInfo, nullptr, &m_ShaderModule));
        LOG_INFO("Created shader module ...");
    }
}
