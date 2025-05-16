#include "VulkanSwapchainUtils.hpp"

#include "Core/Window.hpp"

namespace Engine
{
    vk::Extent2D VulkanSwapchainUtils::ChooseExtent(vk::SurfaceCapabilitiesKHR capabilities)
    {
        // Swapchain dimensions are fixed by the surface. Don't change anything
        if(capabilities.currentExtent.width != std::numeric_limits<u32>::max())
        {
            Engine::Window::SetFramebufferWidth(capabilities.currentExtent.width);
            Engine::Window::SetFramebufferHeight(capabilities.currentExtent.height);
            return capabilities.currentExtent;
        }

        Engine::Window::UpdateFramebufferSize();
        vk::Extent2D extent =
        {
            .width  = Engine::Window::GetFramebufferWidth(),
            .height = Engine::Window::GetFramebufferHeight()
        };

        // Clamp width and height between allowed min and max values of the display manager implementation
        extent.width  = std::clamp(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        extent.height = std::clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return extent;
    }

    vk::SurfaceFormatKHR VulkanSwapchainUtils::ChooseSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& surfaceFormats)
    {
        // Check if prefered format is available
        for(const auto& surfaceFormat : surfaceFormats)
        {
            if(surfaceFormat.format == vk::Format::eB8G8R8A8Srgb &&
               surfaceFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
            {
                return surfaceFormat;
            }
        }

        // If that's not available return the first specified format
        return surfaceFormats[0];
    }

    vk::PresentModeKHR VulkanSwapchainUtils::ChoosePresentMode(const std::vector<vk::PresentModeKHR>& presentModes)
    {
        // Check if mailbox mode aka triple buffering is available
        for(const auto& presentMode : presentModes)
        {
            if(presentMode == vk::PresentModeKHR::eMailbox)
            {
                return presentMode;
            }
        }

        // Queue mode is guaranteed to be available
        return vk::PresentModeKHR::eFifo;
    }

    // Copied from Khronos 'hello_triangle_1_3.cpp' example
    void VulkanSwapchainUtils::TransitionImageLayout
    (
        vk::CommandBuffer       cmd,
        vk::Image               image,
        vk::ImageLayout         oldLayout,
        vk::ImageLayout         newLayout,
        vk::AccessFlags2        srcAccessMask,
        vk::AccessFlags2        dstAccessMask,
        vk::PipelineStageFlags2 srcStage,
        vk::PipelineStageFlags2 dstStage
    )
    {
        // Initialize the VkImageMemoryBarrier2 structure
        vk::ImageMemoryBarrier2 imageMemoryBarrier
        {
            // Specify the pipeline stages and access masks for the barrier
            .srcStageMask  = srcStage,             // Source pipeline stage mask
            .srcAccessMask = srcAccessMask,        // Source access mask
            .dstStageMask  = dstStage,             // Destination pipeline stage mask
            .dstAccessMask = dstAccessMask,        // Destination access mask

            // Specify the old and new layouts of the image
            .oldLayout = oldLayout,        // Current layout of the image
            .newLayout = newLayout,        // Target layout of the image

            // We are not changing the ownership between queues
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,

            // Specify the image to be affected by this barrier
            .image = image,

            // Define the subresource range (which parts of the image are affected)
            .subresourceRange =
            {
                .aspectMask     = vk::ImageAspectFlagBits::eColor,  // Affects the color aspect of the image
                .baseMipLevel   = 0,                                // Start at mip level 0
                .levelCount     = 1,                                // Number of mip levels affected
                .baseArrayLayer = 0,                                // Start at array layer 0
                .layerCount     = 1                                 // Number of array layers affected
            }
        };

        // Initialize the VkDependencyInfo structure
        vk::DependencyInfo dependencyInfo
        {
            .imageMemoryBarrierCount = 1,                    // Number of image memory barriers
            .pImageMemoryBarriers    = &imageMemoryBarrier   // Pointer to the image memory barrier(s)
        };

        // Record the pipeline barrier into the command buffer
        cmd.pipelineBarrier2(&dependencyInfo);
    }
}
