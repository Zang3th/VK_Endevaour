#include "VulkanFrame.hpp"
#include "VulkanAssert.hpp"
#include "VulkanSwapchainUtils.hpp"

namespace Engine
{
    void VulkanFrame::Begin(const SwapchainImage& image, vk::Extent2D extent) const
    {
        // Start command buffer recording
        vk::CommandBufferBeginInfo cmdBeginInfo{};
        VK_VERIFY(CommandBuffer.begin(&cmdBeginInfo));

        // Transition image layout from undefined to color
        VulkanSwapchainUtils::TransitionImageLayout
        (
            CommandBuffer,
            image.Image,
            vk::ImageLayout::eUndefined,
            vk::ImageLayout::eColorAttachmentOptimal,
            vk::AccessFlagBits2::eNone,
            vk::AccessFlagBits2::eColorAttachmentWrite,
            vk::PipelineStageFlagBits2::eTopOfPipe,
            vk::PipelineStageFlagBits2::eColorAttachmentOutput
        );

        // Set up clear value
        vk::ClearValue clearValue { .color = {{{ 1.0f, 1.0f, 1.0f, 1.0f }}}};

        // Set up rendering attachment info
        vk::RenderingAttachmentInfo colorAttachment
        {
            .imageView   = image.View,
            .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
            .loadOp      = vk::AttachmentLoadOp::eClear,
            .storeOp     = vk::AttachmentStoreOp::eStore,
            .clearValue  = clearValue
        };

        // Begin rendering
        vk::RenderingInfo renderingInfo
        {
            .renderArea = { .offset = { .x = 0, .y = 0 }, .extent = extent },
            .layerCount = 1,
            .colorAttachmentCount = 1,
            .pColorAttachments = &colorAttachment
        };

        CommandBuffer.beginRendering(&renderingInfo);
    }

    void VulkanFrame::End(const SwapchainImage& image) const
    {
        // Complete rendering
        CommandBuffer.endRendering();

        // Transition image layout from color to present
        VulkanSwapchainUtils::TransitionImageLayout
        (
            CommandBuffer,
            image.Image,
            vk::ImageLayout::eColorAttachmentOptimal,
            vk::ImageLayout::ePresentSrcKHR,
            vk::AccessFlagBits2::eColorAttachmentWrite,
            vk::AccessFlagBits2::eNone,
            vk::PipelineStageFlagBits2::eColorAttachmentOutput,
            vk::PipelineStageFlagBits2::eBottomOfPipe
        );

        // End command buffer recording
        VK_VERIFY(CommandBuffer.end());
    }
}
