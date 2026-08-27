#pragma once

#include <vulkan/vulkan.hpp>

namespace Engine::Graphics
{
    struct VulkanRenderTargetCreateInfo
    {
        vk::Format           Format;
        vk::Extent2D         Extent;
        vk::ImageUsageFlags  UsageFlags;
        vk::ImageAspectFlags AspectFlags;
    };

    class VulkanRenderTarget
    {
    public:
        VulkanRenderTarget(const VulkanRenderTargetCreateInfo& createInfo, const vk::Device& device);
        ~VulkanRenderTarget();

        VulkanRenderTarget(const VulkanRenderTarget&)            = delete;
        VulkanRenderTarget& operator=(const VulkanRenderTarget&) = delete;

        VulkanRenderTarget(VulkanRenderTarget&& other) noexcept;
        VulkanRenderTarget& operator=(VulkanRenderTarget&& other) noexcept;

        [[nodiscard]] vk::Image     GetImage() const { return m_Image; }
        [[nodiscard]] vk::ImageView GetView() const { return m_View; }

    private:
        void Create(const VulkanRenderTargetCreateInfo& createInfo);
        void Destroy();

        vk::Device    m_Device           = nullptr;
        vk::Image     m_Image            = nullptr;
        vk::ImageView m_View             = nullptr;
        void*         m_AllocationHandle = nullptr;
    };
}
