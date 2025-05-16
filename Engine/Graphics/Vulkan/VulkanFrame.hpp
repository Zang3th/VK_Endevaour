#pragma once

#include <vulkan/vulkan.hpp>

#include "VulkanSwapchainUtils.hpp"

namespace Engine
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
