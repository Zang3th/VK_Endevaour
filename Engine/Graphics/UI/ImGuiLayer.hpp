#include "Graphics/Vulkan/VulkanContext.hpp"
#include "Graphics/Vulkan/VulkanDescriptorPool.hpp"

#include <vulkan/vulkan.hpp>

namespace Engine::Graphics
{
    class ImGuiLayer
    {
    public:
        ImGuiLayer(VulkanContext* context);
        ~ImGuiLayer();

        void BeginFrame();
        void EndFrame(vk::CommandBuffer commandBuffer);

    private:
        void CreateDescriptorPool();
        void Init();
        void Shutdown();

        VulkanContext*              m_Context;
        Scope<VulkanDescriptorPool> m_DescriptorPool;
    };
}
