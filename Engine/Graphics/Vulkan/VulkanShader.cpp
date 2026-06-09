#include "Core/Types.hpp"
#include "Core/Utility.hpp"

#include "Debug/Log.hpp"

#include "Graphics/Vulkan/VulkanAssert.hpp"
#include "Graphics/Vulkan/VulkanShader.hpp"

namespace Engine::Graphics
{
    // ----- Public -----

    VulkanShader::VulkanShader(const vk::Device&            device,
                               vk::ShaderStageFlagBits      stage,
                               const std::filesystem::path& path)
        : m_Device(device), m_Stage(stage)
    {
        std::vector<char> code = Core::Utility::ReadFileAsBytes(path);
        CreateShaderModule(std::move(code));
    }

    VulkanShader::~VulkanShader()
    {
        LOG_INFO("VulkanShader::Destructor(): {} ...", vk::to_string(m_Stage));
        m_Device.destroyShaderModule(m_Module);
    }

    [[nodiscard]] vk::PipelineShaderStageCreateInfo VulkanShader::GetPipelineShaderStageCreateInfo() const
    {
        return { .stage = m_Stage, .module = m_Module, .pName = "main" };
    }

    // ----- Private -----

    void VulkanShader::CreateShaderModule(std::vector<char>&& code)
    {
        const vk::ShaderModuleCreateInfo shaderCreateInfo{ .codeSize = code.size(), .pCode = (u32*)code.data() };

        VK_VERIFY(m_Device.createShaderModule(&shaderCreateInfo, nullptr, &m_Module));
        LOG_INFO("Created shader module ({}) ...", vk::to_string(m_Stage));
    }
}
