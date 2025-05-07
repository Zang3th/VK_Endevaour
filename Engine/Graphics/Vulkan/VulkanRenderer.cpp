#include "VulkanRenderer.hpp"

#include "Debug/Log.hpp"

namespace Engine
{
    // ----- Public -----

    VulkanRenderer::VulkanRenderer()
    {
        m_Context = MakeScope<VulkanContext>();
        m_Device  = m_Context->GetDevice();
    }

    void VulkanRenderer::LoadShader(const std::string& name, vk::ShaderStageFlagBits stage, const std::filesystem::path& path)
    {
        ASSERT(!m_Shaders.contains(name), "Duplicate shader name '{}'. Did not load anything ...", name);
        m_Shaders[name] = MakeScope<VulkanShader>(m_Device->GetHandle(), stage, path);
    }

    void VulkanRenderer::LoadModel(const std::string& name, const std::filesystem::path& path)
    {
        ASSERT(!m_Models.contains(name), "Duplicate model name '{}'. Did not load anything ...", name);
        m_Models[name] = MakeScope<VulkanModel>(m_Device->GetHandle(), path);
    }

    [[nodiscard]] const VulkanShader* VulkanRenderer::GetShader(const std::string& name) const
    {
        const auto it = m_Shaders.find(name);
        ASSERT(it != m_Shaders.end(), "Shader '{}' was not found!", name);
        return it->second.get();
    }
}
