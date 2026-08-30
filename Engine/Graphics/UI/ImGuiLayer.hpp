#pragma once

#include "Core/Memory.hpp"

#include <vulkan/vulkan.hpp>

namespace Engine::Graphics
{
    class VulkanContext;
    class VulkanDescriptorPool;
}

namespace Engine::Graphics
{
    class ImGuiLayer
    {
    public:
        ImGuiLayer(VulkanContext* context);
        ~ImGuiLayer();

        ImGuiLayer(const ImGuiLayer&)            = delete;
        ImGuiLayer& operator=(const ImGuiLayer&) = delete;

        void BeginFrame();
        void RenderFrame(vk::CommandBuffer commandBuffer);

    private:
        void CreateDescriptorPool();
        void Init();
        void ConfigureStyle();

        VulkanContext*              m_Context;
        Scope<VulkanDescriptorPool> m_DescriptorPool;
    };
}
