#pragma once

#include "Graphics/Vulkan/VulkanSwapchainUtils.hpp"

#include <vulkan/vulkan.hpp>

namespace Engine::Graphics
{
    class VulkanFrame
    {
    public:
        void Begin(const SwapchainImage& image, vk::Extent2D extent) const;
        void End(const SwapchainImage& image) const;

        vk::CommandBuffer CommandBuffer  = nullptr;
        vk::Semaphore     ImageAvailable = nullptr;
        vk::Semaphore     RenderFinished = nullptr;
        vk::Fence         InFlight       = nullptr;
        u32               ImageIndex     = UINT32_MAX;
    };
}
