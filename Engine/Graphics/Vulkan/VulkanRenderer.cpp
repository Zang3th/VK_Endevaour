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
        if(!m_Shaders.contains(name))
        {
            m_Shaders[name] = MakeScope<VulkanShader>(stage, path);
            LOG_INFO("Mapped name '{}' to shader '{}'", name, path.string());
            return;
        }

        ASSERT(false, "Duplicate shader name '{}'. Did not load anything ...", name);
    }

    [[nodiscard]] const VulkanShader* VulkanRenderer::GetShader(const std::string& name) const
    {
        const auto it = m_Shaders.find(name);
        if(it != m_Shaders.end())
        {
            return it->second.get();
        }

        ASSERT(false, "Shader '{}' was not found!", name);
        return nullptr;
    }
}
