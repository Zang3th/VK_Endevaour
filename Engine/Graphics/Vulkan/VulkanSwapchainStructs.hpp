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

        u32 MinImageCount = 0;

        // WARN: Hardcoded for now. Will break for values > 1
        [[nodiscard]] constexpr u32 GetColorAttachmentCount() const { return 1; }
    };

    struct SwapchainImage
    {
        // Image handle owned by the swapchain implementation.
        vk::Image Image = nullptr;

        // Engine-owned view into the swapchain image.
        // Used as the color attachment view for dynamic rendering.
        vk::ImageView View = nullptr;

        // Signaled by graphics queue when rendering into this image is complete.
        // Waited by present queue before presentation.
        vk::Semaphore RenderFinished = nullptr;
    };

    struct VulkanFrameResources
    {
        // Reused command buffer for this frame slot.
        // It records commands for whichever swapchain image was acquired this frame.
        vk::CommandBuffer CommandBuffer = nullptr;

        // Synchronizes acquire -> render for this frame slot.
        // Signaled by acquire, waited by graphics queue before rendering.
        vk::Semaphore ImageAvailable = nullptr;

        // CPU waits before reusing this frame slot.
        // Signaled by graphics queue when submitted work for this frame is complete.
        vk::Fence InFlight = nullptr;
    };

    // Transient frame context returned after acquiring a swapchain image.
    // Combines the current frame-in-flight resources with the acquired image index.
    // Valid only for the frame in which it was returned.
    struct SwapchainFrame
    {
        VulkanFrameResources* Resources  = nullptr;
        u32                   ImageIndex = 0;
        u32                   FrameIndex = 0;
        vk::Extent2D          Extent     = { .width = 0, .height = 0 };
    };
}
