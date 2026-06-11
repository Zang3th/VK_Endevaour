#pragma once

#include "Core/Types.hpp"

#include <vulkan/vulkan.hpp>

#include <vector>

namespace Engine::Graphics
{
    // Capabilities and options reported by the physical device for the current surface.
    // Used to choose the concrete swapchain properties during creation/recreation.
    struct SwapchainSupport
    {
        vk::SurfaceCapabilitiesKHR        Capabilities;
        std::vector<vk::SurfaceFormatKHR> Formats;
        std::vector<vk::PresentModeKHR>   PresentModes;

        [[nodiscard]] b8 IsComplete() const { return (!Formats.empty() && !PresentModes.empty()); };
    };

    // These values describe the current swapchain and may change during recreation.
    struct SwapchainProperties
    {
        vk::Extent2D                    Extent        = { .width = 0, .height = 0 };
        vk::SurfaceFormatKHR            SurfaceFormat = { .format     = vk::Format::eUndefined,
                                                          .colorSpace = vk::ColorSpaceKHR::eSrgbNonlinear };
        vk::PresentModeKHR              PresentMode   = vk::PresentModeKHR::eFifo;
        vk::SurfaceTransformFlagBitsKHR Transform     = vk::SurfaceTransformFlagBitsKHR::eIdentity;

        static constexpr u32 ColorAttachmentCount = 1; // WARN: Hardcoded for now. Will break for values > 1

        u32 MinImageCount = 0;
    };

    // Per-swapchain-image resources. The swapchain owns a vector of these.
    struct SwapchainImage
    {
        vk::Image     Image          = nullptr;
        vk::ImageView View           = nullptr;
        vk::Semaphore RenderFinished = nullptr;
    };

    // Per-frame-in-flight CPU/GPU synchronization and command recording resources.
    // This is independent from the acquired swapchain image.
    // The actual swapchain image index is returned by vkAcquireNextImageKHR per frame.
    struct VulkanFrameResources
    {
        vk::CommandBuffer CommandBuffer  = nullptr;
        vk::Semaphore     ImageAvailable = nullptr;
        vk::Fence         InFlight       = nullptr;
    };

    // Transient frame context returned after acquiring a swapchain image.
    // Combines the current frame-in-flight resources with the acquired image index.
    // Valid only for the frame in which it was returned.
    struct SwapchainFrame
    {
        VulkanFrameResources* Resources  = nullptr;
        u32                   ImageIndex = 0;
        vk::Extent2D          Extent     = { .width = 0, .height = 0 };
    };
}
