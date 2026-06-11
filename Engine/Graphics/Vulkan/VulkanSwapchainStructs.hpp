#pragma once

#include "Core/Types.hpp"

#include <vulkan/vulkan.hpp>

#include <vector>

namespace Engine::Graphics
{
    struct SwapchainSupport
    {
        vk::SurfaceCapabilitiesKHR        Capabilities;
        std::vector<vk::SurfaceFormatKHR> Formats;
        std::vector<vk::PresentModeKHR>   PresentModes;

        [[nodiscard]] b8 isComplete() const { return (!Formats.empty() && !PresentModes.empty()); };
    };

    struct SwapchainProperties
    {
        vk::Extent2D                    Extent        = { .width = 0, .height = 0 };
        vk::SurfaceFormatKHR            SurfaceFormat = { .format     = vk::Format::eUndefined,
                                                          .colorSpace = vk::ColorSpaceKHR::eSrgbNonlinear };
        vk::PresentModeKHR              PresentMode   = vk::PresentModeKHR::eFifo;
        vk::SurfaceTransformFlagBitsKHR Transform     = vk::SurfaceTransformFlagBitsKHR::eIdentity;

        static constexpr u32 ColorAttachmentCount = 1; // WARN: Hardcoded for now. Will break for values > 1
                                                       //
        u32 MinImageCount = 0;
    };

    struct SwapchainImage
    {
        vk::Image     Image          = nullptr;
        vk::ImageView View           = nullptr;
        vk::Semaphore RenderFinished = nullptr;
    };

    // Per-frame CPU/GPU synchronization resources.
    // This is independent from the acquired swapchain image.
    // The actual image index is returned by vkAcquireNextImageKHR per frame.
    struct VulkanFrameResources
    {
        vk::CommandBuffer CommandBuffer  = nullptr;
        vk::Semaphore     ImageAvailable = nullptr;
        vk::Fence         InFlight       = nullptr;
    };

    struct SwapchainFrame
    {
        VulkanFrameResources* Resources  = nullptr;
        u32                   ImageIndex = 0;
        vk::Extent2D          Extent     = { .width = 0, .height = 0 };
    };
}
