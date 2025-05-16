#pragma once

#include <vulkan/vulkan.hpp>

#include "Core/Types.hpp"

namespace Engine
{
    struct SwapchainSupport
    {
        vk::SurfaceCapabilitiesKHR        Capabilities;
        std::vector<vk::SurfaceFormatKHR> Formats;
        std::vector<vk::PresentModeKHR>   PresentModes;

        [[nodiscard]] b8 isComplete() const
        {
            return (!Formats.empty() && !PresentModes.empty());
        };
    };

    struct SwapchainProperties
    {
        vk::Extent2D         Extent        = { .width = 0, .height = 0 };
        vk::SurfaceFormatKHR SurfaceFormat = { .format = vk::Format::eUndefined,
                                               .colorSpace = vk::ColorSpaceKHR::eSrgbNonlinear };
        vk::PresentModeKHR   PresentMode   = vk::PresentModeKHR::eFifo;

        vk::SurfaceTransformFlagBitsKHR Transform = vk::SurfaceTransformFlagBitsKHR::eIdentity;

        u32 ImageCount  = 0;
    };

    struct SwapchainImage
    {
        vk::Image     Image = nullptr;
        vk::ImageView View  = nullptr;
    };

    class VulkanSwapchainUtils
    {
        public:
            VulkanSwapchainUtils() = delete;

            static vk::Extent2D ChooseExtent(vk::SurfaceCapabilitiesKHR capabilities);
            static vk::SurfaceFormatKHR ChooseSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& surfaceFormats);
            static vk::PresentModeKHR ChoosePresentMode(const std::vector<vk::PresentModeKHR>& presentModes);
            static void TransitionImageLayout
            (
                vk::CommandBuffer       cmd,
                vk::Image               image,
                vk::ImageLayout         oldLayout,
                vk::ImageLayout         newLayout,
                vk::AccessFlags2        srcAccessMask,
                vk::AccessFlags2        dstAccessMask,
                vk::PipelineStageFlags2 srcStage,
                vk::PipelineStageFlags2 dstStage
            );
    };
}
