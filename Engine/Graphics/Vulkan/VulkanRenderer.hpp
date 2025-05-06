#pragma once

#include "VulkanContext.hpp"
#include "VulkanShader.hpp"

#include <unordered_map>

namespace Engine
{
    class VulkanRenderer
    {
        public:
            VulkanRenderer();

            void LoadShader(const std::string& name, vk::ShaderStageFlagBits stage, const std::filesystem::path& path);

            [[nodiscard]] const VulkanShader* GetShader(const std::string& name) const;

        private:
            Scope<VulkanContext> m_Context;
            const VulkanDevice*  m_Device;

            std::unordered_map<std::string, Scope<VulkanShader>> m_Shaders;
    };
}
