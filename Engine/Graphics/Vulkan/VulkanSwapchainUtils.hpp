#pragma once

#include <vulkan/vulkan.hpp>

namespace Engine::Platform
{
    struct FramebufferExtent;
}

namespace Engine::Graphics
{
    class VulkanSwapchainUtils
    {
    public:
        VulkanSwapchainUtils() = delete;

        [[nodiscard]] static vk::Extent2D                ToVkExtent(const Platform::FramebufferExtent& extent);
        [[nodiscard]] static Platform::FramebufferExtent ToFramebufferExtent(const vk::Extent2D& extent);
        [[nodiscard]] static vk::Extent2D         ChooseExtent(const vk::SurfaceCapabilitiesKHR&  capabilities,
                                                               const Platform::FramebufferExtent& framebufferExtent);
        [[nodiscard]] static vk::SurfaceFormatKHR ChooseSurfaceFormat(
            const std::vector<vk::SurfaceFormatKHR>& surfaceFormats);
        [[nodiscard]] static vk::PresentModeKHR ChoosePresentMode(const std::vector<vk::PresentModeKHR>& presentModes);

        static void TransitionImageLayout(vk::CommandBuffer       cmd,
                                          vk::Image               image,
                                          vk::ImageLayout         oldLayout,
                                          vk::ImageLayout         newLayout,
                                          vk::AccessFlags2        srcAccessMask,
                                          vk::AccessFlags2        dstAccessMask,
                                          vk::PipelineStageFlags2 srcStage,
                                          vk::PipelineStageFlags2 dstStage,
                                          vk::ImageAspectFlags    aspectFlags);
    };
}
