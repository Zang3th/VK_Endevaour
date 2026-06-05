#pragma once

#include "Graphics/Vulkan/VulkanSwapchainUtils.hpp"

#include <vulkan/vulkan.hpp>

namespace Engine::Graphics
{
    // Per-frame CPU/GPU synchronization resources.
    // This is independent from the acquired swapchain image.
    // The actual image index is returned by vkAcquireNextImageKHR per frame.
    class VulkanFrame
    {
    public:
        void Begin(const SwapchainImage& image, vk::Extent2D extent) const;
        void End(const SwapchainImage& image) const;

        vk::CommandBuffer CommandBuffer  = nullptr;
        vk::Semaphore     ImageAvailable = nullptr;
        vk::Fence         InFlight       = nullptr;
    };
}
