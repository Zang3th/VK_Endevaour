#pragma once

#include <vulkan/vulkan.hpp>

namespace Engine::Graphics
{
    class VulkanSwapchainUtils
    {
    public:
        VulkanSwapchainUtils() = delete;

        static vk::Extent2D         ChooseExtent(vk::SurfaceCapabilitiesKHR capabilities);
        static vk::SurfaceFormatKHR ChooseSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& surfaceFormats);
        static vk::PresentModeKHR   ChoosePresentMode(const std::vector<vk::PresentModeKHR>& presentModes);
        static void                 TransitionImageLayout(vk::CommandBuffer       cmd,
                                                          vk::Image               image,
                                                          vk::ImageLayout         oldLayout,
                                                          vk::ImageLayout         newLayout,
                                                          vk::AccessFlags2        srcAccessMask,
                                                          vk::AccessFlags2        dstAccessMask,
                                                          vk::PipelineStageFlags2 srcStage,
                                                          vk::PipelineStageFlags2 dstStage);
    };
}
