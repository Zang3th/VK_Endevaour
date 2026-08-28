#pragma once

#include "Core/Types.hpp"

#include "Graphics/Vulkan/VulkanSwapchainStructs.hpp"

namespace Engine::Graphics
{
    struct QueueFamilyIndices
    {
        u32 GraphicsFamily = UINT32_MAX;
        u32 TransferFamily = UINT32_MAX;

        [[nodiscard]] b8 HasGraphicsFamily() const { return GraphicsFamily != UINT32_MAX; }
        [[nodiscard]] b8 HasTransferFamily() const { return TransferFamily != UINT32_MAX; }
        [[nodiscard]] b8 IsComplete() const
        {
            return ((GraphicsFamily != UINT32_MAX) && (TransferFamily != UINT32_MAX));
        };
    };

    class VulkanPhysicalDevice
    {
    public:
        VulkanPhysicalDevice(const vk::Instance& instance, const vk::SurfaceKHR& surface);
        ~VulkanPhysicalDevice();

        [[nodiscard]] const vk::PhysicalDevice& GetHandle() const { return m_PhysicalDevice; };
        [[nodiscard]] const QueueFamilyIndices& GetQueueFamilies() const { return m_QueueFamilyIndices; };
        [[nodiscard]] SwapchainSupport          GetSwapchainSupport() const
        {
            return QuerySwapchainSupport(m_PhysicalDevice, m_Surface);
        }

    private:
        void                           PickDevice();
        [[nodiscard]] SwapchainSupport QuerySwapchainSupport(const vk::PhysicalDevice& device,
                                                             const vk::SurfaceKHR&     surface) const;

        const vk::Instance&          m_Instance;
        const vk::SurfaceKHR&        m_Surface;
        vk::PhysicalDevice           m_PhysicalDevice;
        QueueFamilyIndices           m_QueueFamilyIndices;
        vk::PhysicalDeviceProperties m_Properties;
    };
}
